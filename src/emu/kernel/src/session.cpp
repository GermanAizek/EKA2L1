/*
 * Copyright (c) 2018 EKA2L1 Team
 * 
 * This file is part of EKA2L1 project
 * (see bentokun.github.com/EKA2L1).
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <kernel/server.h>
#include <kernel/session.h>

#include <kernel/kernel.h>

#include <common/log.h>

namespace eka2l1 {
    namespace service {
        session::session(kernel_system *kern, server_ptr svr, int async_slot_count)
            : kernel_obj(kern, "", kern->crr_process(), kernel::access_type::global_access) 
            , svr(svr)
            , cookie_address(0)
            , headless_(false) {
            obj_type = kernel::object_type::session;

            svr->attach(this);

            if (async_slot_count > 0) {
                msgs_pool.resize(async_slot_count);

                for (auto &msg : msgs_pool) {
                    msg = std::make_pair(true, kern->create_msg(kernel::owner_type::process));
                }
            }
        }

        // Disconnect
        session::~session() {
        }

        ipc_msg_ptr session::get_free_msg() {
            if (msgs_pool.empty()) {
                return kern->create_msg(kernel::owner_type::process);
            }

            auto free_msg_in_pool = std::find_if(msgs_pool.begin(), msgs_pool.end(),
                [](const auto &msg) { return msg.first; });

            if (free_msg_in_pool != msgs_pool.end()) {
                free_msg_in_pool->first = false;
                return free_msg_in_pool->second;
            }

            return ipc_msg_ptr(nullptr);
        }

        void session::set_slot_free(ipc_msg_ptr &msg) {
            if (msgs_pool.empty()) {
                kern->free_msg(msg);
                return;
            }

            auto wrap_msg = std::find_if(msgs_pool.begin(), msgs_pool.end(),
                [&](const auto &wrap_msg) { return wrap_msg.second == msg; });

            if (wrap_msg != msgs_pool.end()) {
                wrap_msg->first = true;
            }
        }

        // This behaves a little different then other
        int session::send_receive_sync(const int function, const ipc_arg &args, eka2l1::ptr<epoc::request_status> request_sts) {
            ipc_msg_ptr &msg = kern->crr_thread()->get_sync_msg();

            if (!msg) {
                return -1;
            }

            msg->function = function;
            msg->args = args;
            msg->own_thr = kern->crr_thread();
            msg->request_sts = request_sts;

            send(msg);

            return 0;
        }

        int session::send_receive(const int function, const ipc_arg &args, eka2l1::ptr<epoc::request_status> request_sts) {
            ipc_msg_ptr msg = get_free_msg();

            if (!msg) {
                return -1;
            }

            msg->function = function;
            msg->args = args;
            msg->request_sts = request_sts;
            msg->own_thr = kern->crr_thread();

            send(msg);

            return 0;
        }

        int session::send(ipc_msg_ptr &msg) {
            server_msg smsg;

            smsg.real_msg = msg;
            smsg.real_msg->msg_status = ipc_message_status::delivered;
            smsg.real_msg->msg_session = (headless_) ? nullptr : this;
            smsg.real_msg->session_ptr_lle = cookie_address;

            return svr->deliver(smsg);
        }

        void session::destroy() {
            // Free the message pool
            for (const auto &msg : msgs_pool) {
                kern->free_msg(msg.second);
            }

            if (!kern->crr_thread()) {
                return;
            }

            // Try to send a disconnect message. Headless session and use sync message.
            headless_ = !svr->is_hle();
            eka2l1::ipc_arg arg;

            arg.flag = 0;
            std::fill(arg.args, arg.args + 4, 0);

            //send_receive_sync(standard_ipc_message_disconnect, arg, 0);
            
            if (svr->is_hle()) {
                svr->process_accepted_msg();
            }
        }
    }
}
