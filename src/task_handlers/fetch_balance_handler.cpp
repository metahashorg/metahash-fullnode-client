#include "fetch_balance_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

fetch_balance_handler::fetch_balance_handler(http_session_ptr session)
    : base_network_handler(settings::server::get_tor(), session)
{
    m_duration.set_message(__func__);
}

bool fetch_balance_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "address", m_addr), "address field not found")
        CHK_PRM(!m_addr.empty(), "address is empty")
        CHK_PRM(m_addr.compare(0, 2, "0x") == 0, "address field incorrect format")

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("address", m_addr.c_str());
        }
        return true;
    }
    END_TRY_RET(false)
}

void fetch_balance_handler::execute() {
    BGN_TRY
    {
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();
            const torrent_node_lib::BalanceInfo balance = sync.getBalance(torrent_node_lib::Address(m_addr));
            balanceInfoToJson(m_addr, balance, sync.getBlockchain().countBlocks(), false, JsonVersion::V1, m_writer.getDoc());
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET()
}
