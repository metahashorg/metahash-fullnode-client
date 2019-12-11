#include "get_block_by_hash_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "cache/blocks_cache.h"
#include "string_utils.h"

get_block_by_hash_handler::get_block_by_hash_handler(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx)
    , m_type(0)
    , m_countTxs(0)
    , m_beginTx(0)
    , m_from_cache(false)
{
    m_duration.set_message(__func__);
    m_name = __func__;
}

bool get_block_by_hash_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "hash", m_hash), "hash field not found")
        CHK_PRM(!m_hash.empty(), "hash is empty")

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("hash", m_hash);
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

        if (blocks_cache::get()->runing()) {
            std::string dump;
            std::string num;
            if (blocks_cache::get()->get_block_by_hash(m_hash, num, dump)) {
                m_writer.reset();
                std::variant<std::monostate, torrent_node_lib::BlockInfo, torrent_node_lib::SignBlockInfo, torrent_node_lib::RejectedTxsBlockInfo> biv = torrent_node_lib::Sync::parseBlockDump(dump, false);
                CHK_PRM(std::holds_alternative<torrent_node_lib::BlockInfo>(biv), "Incorrect block type");
                torrent_node_lib::BlockInfo &bi = std::get<torrent_node_lib::BlockInfo>(biv);
            
                bi.header.blockNumber = std::stoi(num);
                bi.header.countTxs = bi.txs.size();
                if (!settings::system::allowStateBlocks && bi.header.isStateBlock()) {
                    genErrorResponse(-32603, "block " + m_hash + " is a state block and was ignored", m_writer.get_doc());
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

        return true;
    }
    END_TRY(return false)
}

void get_block_by_hash_handler::execute()
{
    BGN_TRY
    {
        if (m_from_cache) {
            LOGDEBUG << "Get block " << m_hash << " from cache";
            return;
        }
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(m_hash);

            if (!settings::system::allowStateBlocks && bh.isStateBlock()) {
                return genErrorResponse(-32603, "block " + m_hash + " is a state block and was ignored", m_writer.get_doc());
            }

            if (!bh.blockNumber.has_value()) {
                return genErrorResponse(-32603, "block " + m_hash + " has not found", m_writer.get_doc());
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
    END_TRY()
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
