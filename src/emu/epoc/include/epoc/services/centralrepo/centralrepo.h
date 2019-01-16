#pragma once

#include <epoc/services/centralrepo/repo.h>
#include <epoc/services/server.h>

#include <map>
#include <string>
#include <unordered_map>

#define CENTRAL_REPO_UID_STRING "10202be9"

namespace eka2l1 {
    class io_system;

    /*! \brief Parse a new centrep ini file.
	 *
	 * \returns False if IO error or invalid centrep configs.
	*/
    bool parse_new_centrep_ini(const std::string &path, central_repo &repo);

    class central_repo_server : public service::server {
        // Cached repos. The key is the owner of the repo.
        std::unordered_map<std::uint32_t, central_repo> repos;
        std::map<std::uint32_t, central_repo_client_session> client_sessions;

        drive_number rom_drv;

        // These drives must be internal, aka not removeable
        std::vector<drive_number> avail_drives;
        std::mutex serv_lock;

        bool first_repo = true;

    protected:
        void rescan_drives(eka2l1::io_system *io);

        eka2l1::central_repo *load_repo(eka2l1::io_system *io, const std::uint32_t key);
        void callback_on_drive_change(eka2l1::io_system *io, const drive_number drv, int act);

    public:
        explicit central_repo_server(eka2l1::system *sys);

        void init(service::ipc_context ctx);

        void create_int(service::ipc_context ctx);
        void create_real(service::ipc_context ctx);
        void create_string8(service::ipc_context ctx);
        void create_string16(service::ipc_context ctx);

        void transaction_start(service::ipc_context ctx);
        void transaction_commit(service::ipc_context ctx);
    };
}