#include "get_block_by_hash_handler_sync.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "settings/settings.h"

bool get_block_by_hash_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")
                
        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")
        
        CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
        CHK_PRM(!hash.empty(), "hash is empty")
        
        auto &jsonParams = *params;
        
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

void get_block_by_hash_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(hash);
    
    if (!bh.blockNumber.has_value()) {
        return genErrorResponse(-32603, "block " + hash + " has not found", m_writer.get_doc());
    }

    if (!settings::system::allowStateBlocks && bh.isStateBlock()) {
        return genErrorResponse(-32603, "block " + hash + " is state block and has been ignored", m_writer.get_doc());
    }
    
    torrent_node_lib::BlockHeader nextBh = sync.getBlockchain().getBlock(*bh.blockNumber + 1);
    std::vector<torrent_node_lib::TransactionInfo> signs;
    if (nextBh.blockNumber.has_value()) {
        const torrent_node_lib::BlockInfo nextBi = sync.getFullBlock(nextBh, 0, 10);
        signs = nextBi.getBlockSignatures();
    }
    if (type == 0 || type == 4) {
        blockHeaderToJson(bh, signs, false, JsonVersion::V1, m_writer.get_doc());
    } else {
        const torrent_node_lib::BlockInfo bi = sync.getFullBlock(bh, beginTx, countTxs);
        blockInfoToJson(bi, signs, type, false, JsonVersion::V1, m_writer.get_doc());
    }
} END_TRY_RET();
}
