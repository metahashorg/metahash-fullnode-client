#include "task_handlers.h"

// get_count_blocks_handler
bool get_count_blocks_handler::prepare_params()
{
    CHK_PRM(m_id, "id field not found")
    return true;
}
