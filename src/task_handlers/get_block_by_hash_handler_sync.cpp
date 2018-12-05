#include "get_block_by_hash_handler_sync.h"

#include "../SyncSingleton.h"

#include "../generate_json.h"

#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool get_block_by_hash_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        
        
        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")
        
        CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
        CHK_PRM(!hash.empty(), "hash is empty")
        
        type = 0;
        auto jValue = this->m_reader.get("type", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "type field incorrect format")
            type = static_cast<uint32_t>(std::stoi(tmp));
            m_writer.add_param("type", type);
        }
                
        countTxs = 0;
        jValue = this->m_reader.get("countTxs", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "countTxs field incorrect format")
            countTxs = std::stoull(tmp);
            m_writer.add_param("countTxs", countTxs);
        }
        
        beginTx = 0;
        jValue = this->m_reader.get("beginTx", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "beginTx field incorrect format")
            beginTx = std::stoull(tmp);
            m_writer.add_param("beginTx", beginTx);
        }
                
        return true;
    }
    END_TRY_RET(false)
}

void get_block_by_hash_handler_sync::executeImpl() {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(hash);
    
    if (!bh.blockNumber.has_value()) {
        return genErrorResponse(-32603, "block " + hash + " not found", m_writer.getDoc());
    }
    
    torrent_node_lib::BlockHeader nextBh = sync.getBlockchain().getBlock(*bh.blockNumber + 1);
    sync.fillSignedTransactionsInBlock(nextBh);
    if (type == 0 || type == 4) {
        blockHeaderToJson(bh, nextBh, false, JsonVersion::V1, m_writer.getDoc());
    } else {
        const torrent_node_lib::BlockInfo bi = sync.getFullBlock(bh, beginTx, countTxs);
        blockInfoToJson(bi, nextBh, type, false, JsonVersion::V1, m_writer.getDoc());
    }
}
