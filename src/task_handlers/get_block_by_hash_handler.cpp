#include "get_block_by_hash_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

get_block_by_hash_handler::get_block_by_hash_handler(http_session_ptr session)
    : base_network_handler(settings::server::get_tor(), session)
    , m_type(0)
    , m_countTxs(0)
    , m_beginTx(0)
{
    m_duration.set_message(__func__);
}

bool get_block_by_hash_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "hash", m_hash), "hash field not found")
        CHK_PRM(!m_hash.empty(), "hash is empty")

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("hash", m_hash);
        }

//        auto &jsonParams = *params;
        
//        int64_t type(0);
//        if (jsonParams.HasMember("type") && jsonParams["type"].IsInt64()) {
//            type = jsonParams["type"].GetInt64();
//            m_writer.add_param("type", type);
//        }

//        int64_t countTxs(0);
//        if (jsonParams.HasMember("countTxs") && jsonParams["countTxs"].IsInt64()) {
//            countTxs = jsonParams["countTxs"].GetInt64();
//            m_writer.add_param("countTxs", countTxs);
//        }
        
//        int64_t beginTx(0);
//        if (jsonParams.HasMember("beginTx") && jsonParams["beginTx"].IsInt64()) {
//            beginTx = jsonParams["beginTx"].GetInt64();
//            m_writer.add_param("beginTx", beginTx);
//        }

        if (m_reader.get_value(*params, "type", m_type) && !settings::system::useLocalDatabase) {
            m_writer.add_param("type", m_type);
        }

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

void get_block_by_hash_handler::execute()
{
    BGN_TRY
    {
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(m_hash);

            if (!settings::system::allowStateBlocks && bh.isStateBlock()) {
                return genErrorResponse(-32603, "block " + m_hash + " is a state block and was ignored", m_writer.getDoc());
            }

            if (!bh.blockNumber.has_value()) {
                return genErrorResponse(-32603, "block " + m_hash + " has not found", m_writer.getDoc());
            }

            torrent_node_lib::BlockHeader nextBh = sync.getBlockchain().getBlock(*bh.blockNumber + 1);
            sync.fillSignedTransactionsInBlock(nextBh);
            if (m_type == 0 || m_type == 4) {
                blockHeaderToJson(bh, nextBh, false, JsonVersion::V1, m_writer.getDoc());
            } else {
                const torrent_node_lib::BlockInfo bi = sync.getFullBlock(bh, m_beginTx, m_countTxs);
                blockInfoToJson(bi, nextBh, m_type, false, JsonVersion::V1, m_writer.getDoc());
            }
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET()
}

void get_block_by_hash_handler::process_response(json_rpc_reader &reader)
{
    base_network_handler::process_response(reader);
    if (!settings::system::allowStateBlocks) {
        auto res = reader.get_result();
        if (res) {
            std::string_view type;
            CHK_PRM(reader.get_value(*res, "type", type), "'type' field not found");
            CHK_PRM(type.compare("state") != 0, "block is state-block and has been ignored");
        }
    }
}
