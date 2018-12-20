#include "get_dump_block_by_number_handler_sync.h"

#include "../SyncSingleton.h"

#include "../generate_json.h"

#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

#include "check.h"

bool get_dump_block_by_number_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        
        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")
        
        auto &jsonParams = *params;
        CHK_PRM(jsonParams.HasMember("number") && jsonParams["number"].IsInt64(), "number field not found")
        number = jsonParams["number"].GetInt64();
        
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
    END_TRY_RET(false)
}

void get_dump_block_by_number_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    try {
        const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(number);
        CHECK(bh.blockNumber.has_value(), "block " + std::to_string(number) + " not found");
        const std::string res = sync.getBlockDump(bh, fromByte, toByte, isHex);
        
        CHECK(!res.empty(), "block " + std::to_string(number) + " not found");
        if (isHex) {
            genBlockDumpJson(res, false, m_writer.getDoc());
        } else {
            //return res;
        }
    } catch (const common::exception &e) {
        genErrorResponse(-32603, e, m_writer.getDoc());
    } catch (const std::exception &e) {
        genErrorResponse(-32603, e.what(), m_writer.getDoc());
    } catch (...) {
        genErrorResponse(-32603, "Unknown error", m_writer.getDoc());
    }
} END_TRY_RET();
}
