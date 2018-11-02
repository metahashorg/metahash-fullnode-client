#ifndef GET_TX_HANDLER_H_
#define GET_TX_HANDLER_H_

#include "network_handler.h"

class get_tx_handler : public base_network_handler, public Perform<get_tx_handler> {
public:
    
    get_tx_handler(http_session_ptr session): base_network_handler(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_tx_handler() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // GET_TX_HANDLER_H_
