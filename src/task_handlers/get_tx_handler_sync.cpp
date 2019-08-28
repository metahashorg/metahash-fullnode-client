#include "get_tx_handler_sync.h"
#include "common/convertStrings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool get_tx_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")
        
        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")
        
        CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
        CHK_PRM(!hash.empty(), "hash is empty")
                
        return true;
    }
    END_TRY(return false)
}

void get_tx_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const std::vector<unsigned char> hash2 = common::fromHex(hash);
    const torrent_node_lib::TransactionInfo tx = sync.getTransaction(std::string(hash2.begin(), hash2.end()));
    
    transactionToJson(tx, sync.getBlockchain(), sync.getBlockchain().countBlocks(), sync.getKnownBlock(), false, JsonVersion::V1, m_writer.get_doc());
} END_TRY();
}
