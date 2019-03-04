#ifndef __FETCH_HISTORY_HANDLER_H__
#define __FETCH_HISTORY_HANDLER_H__

#include "network_handler.h"
#include "settings/settings.h"

class fetch_history_handler : public base_network_handler
{
public:
    fetch_history_handler(http_session_ptr session)
        : base_network_handler(settings::server::tor, session) {
        m_duration.set_message(__func__);
    }
    
    virtual ~fetch_history_handler() override {}

protected:
    virtual bool prepare_params() override;
};

#endif // __FETCH_HISTORY_HANDLER_H__
