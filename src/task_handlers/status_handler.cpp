#include "status_handler.h"
#include "settings/settings.h"
#include "cmake_modules/GitSHA1.h"
//#include "cpplib_open_ssl_decor/crypto.h"
#include "common/filesystem_utils.h"
#include <sys/types.h>
#include <dirent.h>
#include "../SyncSingleton.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool status_handler::prepare_params()
{
    BGN_TRY
    {
        std::string_view cmd;
        if (auto params = m_reader.get_params()) {
            if (m_reader.get_value(*params, "cmd", cmd)) {
                if (cmd == "keys") {
                    m_cmd = cmd::keys;
                }
            }
        }
        return true;
    }
    END_TRY_RET(false)
}

void status_handler::execute()
{
    BGN_TRY
    {
        switch (m_cmd) {
            case cmd::general:
            m_writer.add_result("version", "v0");
            m_writer.add_result("git_hash", g_GIT_SHA1);
            m_writer.add_result("network_tor_name", settings::server::torName);
            m_writer.add_result("network_proxy_name", settings::server::proxyName);
            m_writer.add_result("network_tor", settings::server::tor);
            m_writer.add_result("network_proxy", settings::server::proxy);
            m_writer.add_result("use_local_database", settings::system::useLocalDatabase);
            m_writer.add_result("allow_state_blocks", settings::system::allowStateBlocks);
            m_writer.add_result("jrpc_timeout", settings::system::jrpc_timeout);
            m_writer.add_result("jrpc_conn_timeout", settings::system::jrpc_conn_timeout);
            if (syncSingleton() != nullptr) {
                const torrent_node_lib::Sync &sync = *syncSingleton();
                m_writer.add_result("blocks_count", sync.getBlockchain().countBlocks());
                m_writer.add_result("last_block", sync.getKnownBlock());
            }
            break;

        case cmd::keys:
        {
            rapidjson::Value arr(rapidjson::Type::kArrayType);
            if (fs_utils::dir::is_exists(settings::system::wallet_stotage.c_str())) {

                DIR* dirp = opendir(settings::system::wallet_stotage.c_str());
                struct dirent * dp;
                while ((dp = readdir(dirp)) != NULL) {
                    if (dp->d_type != DT_REG) {
                        continue;
                    }
                    std::string_view file = dp->d_name;
                    if (file.rfind(".raw.pub") == std::string_view::npos) {
                        continue;
                    }
                    file.remove_suffix(8);
                    arr.PushBack(rapidjson::Value().SetString(file.data(), static_cast<unsigned>(file.size()), m_writer.get_allocator()),
                                 m_writer.get_allocator());
                }
                closedir(dirp);
            }
            m_writer.add_result("count", arr.Size());
            m_writer.get_value(m_writer.getDoc(), "result", rapidjson::Type::kObjectType).AddMember("keys", arr, m_writer.get_allocator());
            break;
        }

        default:
            m_writer.set_error(1, "unrecognized command");
            break;
        }
    }
    END_TRY
}
