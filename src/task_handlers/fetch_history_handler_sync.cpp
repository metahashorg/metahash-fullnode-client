#include "fetch_history_handler_sync.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool fetch_history_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")
        
        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")
        
        std::string addr;
        CHK_PRM(m_reader.get_value(*params, "address", addr), "address field not found")
        CHK_PRM(!addr.empty(), "address is empty")
        CHK_PRM(addr.compare(0, 2, "0x") == 0, "address field incorrect format")
        
        address = addr.c_str();
        
        auto &jsonParams = *params;
               
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

void fetch_history_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const std::vector<torrent_node_lib::TransactionInfo> txs = sync.getTxsForAddress(torrent_node_lib::Address(address), beginTx, countTxs, countTxs);
    
    addressesInfoToJson(address, txs, sync.getBlockchain(), 0, false, JsonVersion::V1, m_writer.get_doc());
} END_TRY_RET();
}
