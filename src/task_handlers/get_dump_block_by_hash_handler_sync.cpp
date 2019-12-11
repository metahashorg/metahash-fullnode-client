#include "get_dump_block_by_hash_handler_sync.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "check.h"

bool get_dump_block_by_hash_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")
        
        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")
        
        CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
        CHK_PRM(!hash.empty(), "hash is empty")

        auto &jsonParams = *params;
        fromByte = 0;
        toByte = std::numeric_limits<size_t>::max();
        if (jsonParams.HasMember("fromByte") && jsonParams["fromByte"].IsInt64()) {
            fromByte = jsonParams["fromByte"].GetInt64();
        }
        if (jsonParams.HasMember("toByte") && jsonParams["toByte"].IsInt64()) {
            toByte = jsonParams["toByte"].GetInt64();
        }
        
        isHex = true;
        
        return true;
    }
    END_TRY(return false)
}

void get_dump_block_by_hash_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    try {
        const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(hash);
        CHECK(bh.blockNumber.has_value(), "block " + hash + " not found");
        const std::string res = sync.getBlockDump(torrent_node_lib::CommonMimimumBlockHeader(bh.hash, bh.filePos), fromByte, toByte, isHex, false);
        
        CHECK(!res.empty(), "block " + hash + " not found");
        if (isHex) {
            genBlockDumpJson(res, false, m_writer.get_doc());
        } else {
            //return res;
        }
    } catch (const common::exception &e) {
        genErrorResponse(-32603, e, m_writer.get_doc());
    } catch (const std::exception &e) {
        genErrorResponse(-32603, e.what(), m_writer.get_doc());
    } catch (...) {
        genErrorResponse(-32603, "Unknown error", m_writer.get_doc());
    }
} END_TRY();
}
