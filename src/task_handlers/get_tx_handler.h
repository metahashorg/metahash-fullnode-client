#ifndef __GET_TX_HANDLER_H__
#define __GET_TX_HANDLER_H__

#include "network_handler.h"
#include "settings/settings.h"

class get_tx_handler : public base_network_handler
{
public:
    get_tx_handler(http_session_ptr session)
        : base_network_handler(settings::server::tor, session) {
        m_duration.set_message(__func__);
    }
    virtual ~get_tx_handler() override {}

protected:
    virtual bool prepare_params() override;
};

#endif //

