#include "task_handlers.h"

// get_last_txs_handler
bool get_last_txs_handler::prepare_params()
{
    CHK_PRM(m_id, "id field not found")
    return true;
}
