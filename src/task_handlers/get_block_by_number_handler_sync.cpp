#include "get_block_by_number_handler_sync.h"

#include "../SyncSingleton.h"

#include "../generate_json.h"

#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool get_block_by_number_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        
        
        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")
        
        auto &jsonParams = *params;
        CHK_PRM(jsonParams.HasMember("number") && jsonParams["number"].IsInt64(), "number field not found")
        number = jsonParams["number"].GetInt64();
                
        type = 0;
        if (jsonParams.HasMember("type") && jsonParams["type"].IsInt64()) {
            type = jsonParams["type"].GetInt64();
        }
        
        countTxs = 0;
        if (jsonParams.HasMember("countTxs") && jsonParams["countTxs"].IsInt64()) {
            countTxs = jsonParams["countTxs"].GetInt64();
        }
        
        beginTx = 0;
        if (jsonParams.HasMember("beginTx") && jsonParams["beginTx"].IsInt64()) {
            beginTx = jsonParams["beginTx"].GetInt64();
        }
                        
        return true;
    }
    END_TRY_RET(false)
}

void get_block_by_number_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(number);
    
    if (!bh.blockNumber.has_value()) {
        return genErrorResponse(-32603, "block " + std::to_string(number) + " not found", m_writer.getDoc());
    }
    
    torrent_node_lib::BlockHeader nextBh = sync.getBlockchain().getBlock(*bh.blockNumber + 1);
    sync.fillSignedTransactionsInBlock(nextBh);
    if (type == 0 || type == 4) {
        blockHeaderToJson(bh, nextBh, false, JsonVersion::V1, m_writer.getDoc());
    } else {
        const torrent_node_lib::BlockInfo bi = sync.getFullBlock(bh, beginTx, countTxs);
        blockInfoToJson(bi, nextBh, type, false, JsonVersion::V1, m_writer.getDoc());
    }
} END_TRY_RET();
}
