#ifndef GET_DUMP_BLOCK_BY_NUMBER_H_
#define GET_DUMP_BLOCK_BY_NUMBER_H_

#include "network_handler.h"

#include "settings/settings.h"

class get_dump_block_by_number : public base_network_handler, public Perform<get_dump_block_by_number> {
public:
    
    get_dump_block_by_number(http_session_ptr session): base_network_handler(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_dump_block_by_number() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // GET_DUMP_BLOCK_BY_NUMBER_H_
