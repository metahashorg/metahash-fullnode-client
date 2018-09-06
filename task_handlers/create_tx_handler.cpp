#include "create_tx_handler.h"

// create_tx_handler
bool create_tx_handler::get_nonce(mh_count_t& result)
{
    return m_reader.get_value(*m_reader.get_params(), "nonce", result);
}
