#include "task_handlers.h"

// get_last_txs_handler
bool get_last_txs_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        return true;
    }
    END_TRY_RET(false)
}
