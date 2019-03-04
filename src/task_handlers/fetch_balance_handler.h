#ifndef __FETCH_BALANCE_HANDLER_H__
#define __FETCH_BALANCE_HANDLER_H__

#include "network_handler.h"
#include "settings/settings.h"

class fetch_balance_handler : public base_network_handler
{
public:
    fetch_balance_handler(http_session_ptr session)
        : base_network_handler(settings::server::tor, session) {
        m_duration.set_message(__func__);
    }
    virtual ~fetch_balance_handler() override {}

protected:
    virtual bool prepare_params() override;
};

#endif // __FETCH_BALANCE_HANDLER_H__
