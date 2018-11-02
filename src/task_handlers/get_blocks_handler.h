#ifndef GET_BLOCKS_HANDLER_H_
#define GET_BLOCKS_HANDLER_H_

#include "network_handler.h"

class get_blocks_handler : public base_network_handler<get_blocks_handler> {
public:
    typedef base_network_handler<get_blocks_handler> base;
    
    get_blocks_handler(http_session_ptr session): base(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_blocks_handler() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // GET_BLOCKS_HANDLER_H_
