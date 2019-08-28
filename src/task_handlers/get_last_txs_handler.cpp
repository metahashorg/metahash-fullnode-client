#include "get_last_txs_handler.h"
#include "settings/settings.h"

get_last_txs_handler::get_last_txs_handler(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx) {
    m_duration.set_message(__func__);
    m_name = __func__;
}

bool get_last_txs_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")
        return true;
    }
    END_TRY(return false)
}
