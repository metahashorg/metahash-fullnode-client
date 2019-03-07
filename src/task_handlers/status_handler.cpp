#include "status_handler.h"
#include "../wallet_storage/wallet_storage.h"

#include "settings/settings.h"

#include "cmake_modules/GitSHA1.h"

#include "cpplib_open_ssl_decor/crypto.h"

bool status_handler::prepare_params()
{
    BGN_TRY
    {
        return true;
    }
    END_TRY_RET(false)
}

void status_handler::execute()
{
    BGN_TRY
    {
        m_writer.add_result("version", "v0");
        m_writer.add_result("git_hash", g_GIT_SHA1);
        m_writer.add_result("network_tor_name", settings::server::torName);
        m_writer.add_result("network_proxy_name", settings::server::proxyName);
        m_writer.add_result("network_tor", settings::server::tor);
        m_writer.add_result("network_proxy", settings::server::proxy);

        m_writer.add_result("allow_state_blocks", settings::system::allowStateBlocks);
        m_writer.add_result("jrpc_timeout", settings::system::jrpc_timeout);
        m_writer.add_result("jrpc_conn_timeout", settings::system::jrpc_conn_timeout);
    }
    END_TRY
}
