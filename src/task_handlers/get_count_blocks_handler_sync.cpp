#include "get_count_blocks_handler_sync.h"

#include "../SyncSingleton.h"

#include "../generate_json.h"

#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool get_count_blocks_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        
        return true;
    }
    END_TRY_RET(false)
}

void get_count_blocks_handler_sync::executeImpl() {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const size_t countBlocks = sync.getBlockchain().countBlocks();
    
    genCountBlockJson(countBlocks, false, JsonVersion::V1, m_writer.getDoc());
}
