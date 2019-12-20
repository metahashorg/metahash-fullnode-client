#include "get_block_base.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "../sync/synchronize_blockchain.h"
#include "cache/blocks_cache.h"
#include "string_utils.h"

get_block_base::get_block_base(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx)
    , m_type(0)
    , m_countTxs(0)
    , m_beginTx(0)
{
}

bool get_block_base::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        const rapidjson::Value* params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        set_block_id(params);

        if (m_reader.get_value(*params, "countTxs", m_countTxs) && !settings::system::useLocalDatabase) {
            m_writer.add_param("countTxs", m_countTxs);
        }

        if (m_reader.get_value(*params, "beginTx", m_beginTx) && !settings::system::useLocalDatabase) {
            m_writer.add_param("beginTx", m_beginTx);
        }

        const rapidjson::Value* block_type = m_reader.get("type", *params);
        if (block_type) {
            if (block_type->IsString()) {
                if (!settings::system::useLocalDatabase) {
                    m_writer.add_param("type", block_type->GetString());
                }
            } else if (block_type->IsUint()) {
                m_type = block_type->GetUint();
                if (!settings::system::useLocalDatabase) {
                    m_writer.add_param("type", m_type);
                }
            }
        }
        
        return true;
    }
    END_TRY(return false)
}

void get_block_base::execute()
{
    BGN_TRY
    {
        if (blocks_cache::get()->runing()) {
            std::string dump;
            std::string num;
            if ((std::holds_alternative<std::size_t>(m_block_id) &&
                blocks_cache::get()->get_block_by_num(std::get<std::size_t>(m_block_id), dump)) ||
                (std::holds_alternative<std::string>(m_block_id) &&
                 blocks_cache::get()->get_block_by_hash(std::get<std::string>(m_block_id), num, dump)))
            {
                m_writer.reset();
                auto some_block = torrent_node_lib::Sync::parseBlockDump(dump, false);
                CHK_PRM(std::holds_alternative<torrent_node_lib::BlockInfo>(some_block), "Incorrect block type");
                torrent_node_lib::BlockInfo &bi = std::get<torrent_node_lib::BlockInfo>(some_block);
                bi.header.blockNumber = std::holds_alternative<std::size_t>(m_block_id) ? std::get<std::size_t>(m_block_id) :
                                        static_cast<std::size_t>(std::atoi(std::get<std::string>(m_block_id).c_str()));
                bi.header.countTxs = bi.txs.size();
                if (!settings::system::allowStateBlocks && bi.header.isStateBlock()) {
                    genErrorResponse(-32603, "The block " + get_block_id() + " is a state block and has been ignored", m_writer.get_doc());
                    return;
                }
                dump.clear();
                AnyTxInfo txs;
                if (blocks_cache::get()->get_extra_block_for(bi.header.blockNumber.value(), dump)) {
                    auto extra_block = torrent_node_lib::Sync::parseBlockDump(dump, false);
                    if (std::holds_alternative<torrent_node_lib::SignBlockInfo>(extra_block)) {
                        torrent_node_lib::SignBlockInfo& sbi = std::get<torrent_node_lib::SignBlockInfo>(extra_block);
                        txs = std::move(sbi.txs);
                    }
                }
                switch (m_type) {
                case 0:
                case 4:
                    blockHeaderToJson(bi.header, txs, false, JsonVersion::V1, m_writer.get_doc());
                    break;
                default:
                    blockInfoToJson(bi, txs, m_type, false, JsonVersion::V1, m_writer.get_doc());
                    break;
                }
                LOGINFO << "Get block #" << get_block_id() << " from cache";
            } else {
                std::size_t number = static_cast<std::size_t>(std::atoi(get_block_id().c_str()));
                CHK_PRM(number <= blocks_cache::extra_blocks_epoch, "The block does not exists or have not signed yet");
            }
        } else if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            const torrent_node_lib::BlockHeader bh = std::holds_alternative<std::size_t>(m_block_id) ?
                        sync.getBlockchain().getBlock(std::get<std::size_t>(m_block_id)) :
                        sync.getBlockchain().getBlock(std::get<std::string>(m_block_id));

            if (!settings::system::allowStateBlocks && bh.isStateBlock()) {
                return genErrorResponse(-32603, "block " + get_block_id() + " is a state block and was ignored", m_writer.get_doc());
            }

            if (!bh.blockNumber.has_value()) {
                return genErrorResponse(-32603, "block " + get_block_id() + " has not found", m_writer.get_doc());
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

void get_block_base::process_response(json_rpc_reader &reader)
{
    base_network_handler::process_response(reader);
    if (!settings::system::allowStateBlocks) {
        const rapidjson::Value* res = reader.get_result();
        if (res) {
            std::string_view type;
            if (m_reader.get_value(*res, "type", type)) {
                CHK_PRM(type.compare("state") != 0, "block is state-block and has been ignored")
            }
        }
    }
}

std::string get_block_base::get_block_id() const
{
    if (std::holds_alternative<std::size_t>(m_block_id)) {
        return std::to_string(std::get<std::size_t>(m_block_id));
    } else if (std::holds_alternative<std::string>(m_block_id)) {
        return std::get<std::string>(m_block_id);
    }
    return std::string("UNKNOWN");
}
