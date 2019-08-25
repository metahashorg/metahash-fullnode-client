#include "fetch_balance_handler_sync.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool fetch_balance_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        
        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")
        
        std::string addr;
        CHK_PRM(m_reader.get_value(*params, "address", addr), "address field not found")
        CHK_PRM(!addr.empty(), "address is empty")
        CHK_PRM(addr.compare(0, 2, "0x") == 0, "address field incorrect format")
        
        address = addr.c_str();
        
        return true;
    }
    END_TRY_RET(false)
}

void fetch_balance_handler_sync::executeImpl() {
BGN_TRY {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const torrent_node_lib::BalanceInfo balance = sync.getBalance(torrent_node_lib::Address(address));
    
    balanceInfoToJson(address, balance, sync.getBlockchain().countBlocks(), false, JsonVersion::V1, m_writer.get_doc(), false);
} END_TRY_RET();
}
