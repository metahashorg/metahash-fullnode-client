#include "fetch_history_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

fetch_history_handler::fetch_history_handler(http_session_ptr session)
    : base_network_handler(settings::server::get_tor(), session)
    , m_countTxs(0)
    , m_beginTx(0)
{
    m_duration.set_message(__func__);
}

bool fetch_history_handler::prepare_params()
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

        /*
        auto &jsonParams = *params;
        
        uint64_t countTxs(0);
        if (jsonParams.HasMember("countTxs") && jsonParams["countTxs"].IsInt64()) {
            countTxs = jsonParams["countTxs"].GetInt64();
            m_writer.add_param("countTxs", countTxs);
        }
        
        mh_count_t beginTx(0);
        if (jsonParams.HasMember("beginTx") && jsonParams["beginTx"].IsInt64()) {
            beginTx = jsonParams["beginTx"].GetInt64();
            m_writer.add_param("beginTx", beginTx);
        }
        */

        if (m_reader.get_value(*params, "countTxs", m_countTxs) && !settings::system::useLocalDatabase) {
            m_writer.add_param("countTxs", m_countTxs);
        }

        if (m_reader.get_value(*params, "beginTx", m_beginTx) && !settings::system::useLocalDatabase) {
            m_writer.add_param("beginTx", m_beginTx);
        }

        return true;
    }
    END_TRY_RET(false)
}

void fetch_history_handler::execute()
{
    BGN_TRY
    {
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();
            const std::vector<torrent_node_lib::TransactionInfo> txs = sync.getTxsForAddress(torrent_node_lib::Address(m_addr), m_beginTx, m_countTxs);
            addressesInfoToJson(m_addr, txs, sync.getBlockchain(), 0, false, JsonVersion::V1, m_writer.getDoc());
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET();
}
