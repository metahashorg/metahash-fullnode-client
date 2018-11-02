#ifndef FETCH_BALANCE_HANDLER_H_
#define FETCH_BALANCE_HANDLER_H_

#include "network_handler.h"

class fetch_balance_handler : public base_network_handler<fetch_balance_handler> {
public:
    typedef base_network_handler<fetch_balance_handler> base;
    
    fetch_balance_handler(http_session_ptr session): base(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~fetch_balance_handler() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // FETCH_BALANCE_HANDLER_H_
