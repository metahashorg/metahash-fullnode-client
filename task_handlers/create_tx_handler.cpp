#include "task_handlers.h"
#include "create_tx_handler.h"

// create_tx_handler
bool create_tx_handler::get_nonce(mh_count_t&)
{
    CHK_PRM(false, "nonce should be must set")
}
