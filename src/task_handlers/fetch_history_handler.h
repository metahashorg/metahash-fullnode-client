#ifndef FETCH_HISTORY_HANDLER_H_
#define FETCH_HISTORY_HANDLER_H_

#include "network_handler.h"

class fetch_history_handler : public base_network_handler, public Perform<fetch_history_handler> {
public:
    
    fetch_history_handler(http_session_ptr session): base_network_handler(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~fetch_history_handler() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // FETCH_HISTORY_HANDLER_H_
