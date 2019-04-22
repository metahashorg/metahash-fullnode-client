#include "status_handler.h"
#include "settings/settings.h"
#include "cmake_modules/GitSHA1.h"
//#include "cpplib_open_ssl_decor/crypto.h"
#include "common/filesystem_utils.h"
#include "common/string_utils.h"
#include <sys/types.h>
#include <dirent.h>
#include "../SyncSingleton.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "http_session.h"
#include "http_json_rpc_request.h"

#include "version.h"

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
            m_writer.add_result("version", get_version());
            m_writer.add_result("git_hash", g_GIT_SHA1);
            m_writer.add_result("git_date", g_GIT_DATE);
            m_writer.add_result("network_tor_name", settings::server::torName);
            m_writer.add_result("network_proxy_name", settings::server::proxyName);
            m_writer.add_result("network_tor", settings::server::get_tor());
            m_writer.add_result("network_proxy", settings::server::get_proxy());
            m_writer.add_result("use_local_database", settings::system::useLocalDatabase);
            m_writer.add_result("allow_state_blocks", settings::system::allowStateBlocks);
            m_writer.add_result("jrpc_timeout", settings::system::jrpc_timeout);
            m_writer.add_result("jrpc_conn_timeout", settings::system::jrpc_conn_timeout);
            m_writer.add_result("conn_pool_enable", settings::system::conn_pool_enable);
            m_writer.add_result("conn_pool_ttl", settings::system::conn_pool_ttl);
            m_writer.add_result("conn_pool_capacity", settings::system::conn_pool_capacity);
            if (syncSingleton() != nullptr) {
                const torrent_node_lib::Sync &sync = *syncSingleton();
                m_writer.add_result("blocks_count", sync.getBlockchain().countBlocks());
                m_writer.add_result("last_block", sync.getKnownBlock());
            } else {
                asio::io_context io;
                auto request = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), io);
                request->set_path("get-count-blocks");
                request->set_body("{\"id\":1}");
                request->execute();
                std::string result = request->get_result();
                m_writer.add_result("blocks_count", "n/a");
                for (;;) {
                    if (result.empty()) break;
                    json_rpc_reader reader;
                    if (!reader.parse(result)) {
                        m_writer.add_result("blocks_count",
                                            string_utils::str_concat("parse error: ", std::to_string(reader.get_parse_error().Code())));
                        break;
                    }
                    auto tmp = reader.get_error();
                    if (tmp) {
                        m_writer.add_result("blocks_count",
                                            string_utils::str_concat("error: ", reader.stringify(tmp)));
                        break;
                    }
                    tmp = reader.get_result();
                    if (!tmp) {
                        m_writer.add_result("blocks_count", "no result occurred");
                        break;
                    }
                    uint64_t count_blocks = 0;
                    if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                        m_writer.add_result("blocks_count", "'count_blocks' has not found");
                        break;
                    }
                    m_writer.add_result("blocks_count", count_blocks);
                    break;
                }
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
