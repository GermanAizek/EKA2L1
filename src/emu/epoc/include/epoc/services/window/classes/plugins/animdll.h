/*
 * Copyright (c) 2019 EKA2L1 Team
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

#pragma once

#include <epoc/services/window/classes/wsobj.h>
#include <cstdint>

namespace eka2l1::epoc {
    struct anim_dll : public window_client_obj {
        // Nothing yet
        anim_dll(window_server_client_ptr client)
            : window_client_obj(client) {
        }

        std::uint32_t user_count{ 0 };

        void execute_command(service::ipc_context &context, ws_cmd cmd) override;
    };
}