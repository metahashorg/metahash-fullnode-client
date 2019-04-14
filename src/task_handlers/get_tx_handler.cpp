#include "get_tx_handler.h"
#include "settings/settings.h"
#include "common/convertStrings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

get_tx_handler::get_tx_handler(http_session_ptr session)
    : base_network_handler(settings::server::get_tor(), session) {
    m_duration.set_message(__func__);
}

bool get_tx_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "hash", m_hash), "hash field not found")
        CHK_PRM(!m_hash.empty(), "hash is empty")

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("hash", m_hash.c_str());
        }

        return true;
    }
    END_TRY_RET(false)
}

void get_tx_handler::execute()
{
    BGN_TRY
    {
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            const std::vector<unsigned char> hash2 = common::fromHex(m_hash);
            const torrent_node_lib::TransactionInfo tx = sync.getTransaction(std::string(hash2.begin(), hash2.end()));

            transactionToJson(tx, sync.getBlockchain(), sync.getBlockchain().countBlocks(), sync.getKnownBlock(),
                              false, JsonVersion::V1, m_writer.getDoc());
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET()
}
