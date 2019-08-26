#include "get_block_by_number_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "../sync/synchronize_blockchain.h"
#include "cache/blocks_cache.h"
#include "string_utils.h"

extern std::unique_ptr<blocks_cache> g_cache;

get_block_by_number_handler::get_block_by_number_handler(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx)
    , m_number(0)
    , m_type(0)
    , m_countTxs(0)
    , m_beginTx(0)
    , m_from_cache(false)
{
    m_duration.set_message(__func__);
}

bool get_block_by_number_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "number", m_number), "number field not found")

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("number", m_number);
        }

        if (m_reader.get_value(*params, "type", m_type) && !settings::system::useLocalDatabase) {
            m_writer.add_param("type", m_type);
        }

        if (m_reader.get_value(*params, "countTxs", m_countTxs) && !settings::system::useLocalDatabase) {
            m_writer.add_param("countTxs", m_countTxs);
        }

        if (m_reader.get_value(*params, "beginTx", m_beginTx) && !settings::system::useLocalDatabase) {
            m_writer.add_param("beginTx", m_beginTx);
        }

        if (g_cache && g_cache->runing()) {
            std::string dump;
            if (g_cache->get_block_by_num(static_cast<unsigned int>(m_number), dump)) {
                m_writer.reset();
                torrent_node_lib::BlockInfo bi = torrent_node_lib::Sync::parseBlockDump(dump, false);
                bi.header.blockNumber = m_number;
                bi.header.countTxs = bi.txs.size();
                if (!settings::system::allowStateBlocks && bi.header.isStateBlock()) {
                    genErrorResponse(-32603, "block " + std::to_string(m_number) + " is a state block and was ignored", m_writer.get_doc());
                    return false;
                }
                switch (m_type) {
                case 0:
                case 4:
                    blockHeaderToJson(bi.header, {}, false, JsonVersion::V1, m_writer.get_doc());
                    break;
                default:
                    blockInfoToJson(bi, {}, m_type, false, JsonVersion::V1, m_writer.get_doc());
                    break;
                }
                m_from_cache = true;
                return true;
            }
        }

//        auto &jsonParams = *params;
//        CHK_PRM(jsonParams.HasMember("number") && jsonParams["number"].IsInt64(), "number field not found")
//        int64_t number = jsonParams["number"].GetInt64();
//        m_writer.add_param("number", number);
        
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
        
        return true;
    }
    END_TRY_RET(false)
}

void get_block_by_number_handler::execute()
{
    BGN_TRY
    {
        if (m_from_cache) {
            LOGDEBUG << "Get block #" << m_number << " from cache";
            return;
        }
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(m_number);

            if (!settings::system::allowStateBlocks && bh.isStateBlock()) {
                return genErrorResponse(-32603, "block " + std::to_string(m_number) + " is a state block and was ignored", m_writer.get_doc());
            }

            if (!bh.blockNumber.has_value()) {
                return genErrorResponse(-32603, "block " + std::to_string(m_number) + " has not found", m_writer.get_doc());
            }

            torrent_node_lib::BlockHeader nextBh = sync.getBlockchain().getBlock(*bh.blockNumber + 1);
            std::vector<torrent_node_lib::TransactionInfo> signs;
            if (nextBh.blockNumber.has_value()) {
                const torrent_node_lib::BlockInfo nextBi = sync.getFullBlock(nextBh, 0, 10);
                signs = nextBi.getBlockSignatures();
            }
            
            if (m_type == 0 || m_type == 4) {
                blockHeaderToJson(bh, signs, false, JsonVersion::V1, m_writer.get_doc());
            } else {
                const torrent_node_lib::BlockInfo bi = sync.getFullBlock(bh, m_beginTx, m_countTxs);
                blockInfoToJson(bi, signs, m_type, false, JsonVersion::V1, m_writer.get_doc());
            }
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET()
}

void get_block_by_number_handler::process_response(json_rpc_reader &reader)
{
    base_network_handler::process_response(reader);
    if (!settings::system::allowStateBlocks) {
        auto res = reader.get_result();
        if (res) {
            std::string_view type;
            CHK_PRM(m_reader.get_value(*res, "type", type), "'type' field not found")
            CHK_PRM(type.compare("state") != 0, "block is state-block and has been ignored")
        }
    }
}
