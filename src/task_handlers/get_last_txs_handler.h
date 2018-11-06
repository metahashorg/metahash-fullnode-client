#ifndef GET_LAST_TXS_HANDLER_H_
#define GET_LAST_TXS_HANDLER_H_

#include "network_handler.h"

#include "settings/settings.h"

class get_last_txs_handler : public base_network_handler, public Perform<get_last_txs_handler> {
public:
    
    get_last_txs_handler(http_session_ptr session): base_network_handler(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_last_txs_handler() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // GET_LAST_TXS_HANDLER_H_
