#ifndef GET_BLOCK_BY_NUMBER_HANDLER_H_
#define GET_BLOCK_BY_NUMBER_HANDLER_H_

#include "network_handler.h"

class get_block_by_number_handler : public base_network_handler<get_block_by_number_handler> {
public:
    typedef base_network_handler<get_block_by_number_handler> base;
    
    get_block_by_number_handler(http_session_ptr session): base(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_block_by_number_handler() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // GET_BLOCK_BY_NUMBER_HANDLER_H_
