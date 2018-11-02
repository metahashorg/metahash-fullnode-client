#ifndef GET_DUMP_BLOCK_BY_HASH_H_
#define GET_DUMP_BLOCK_BY_HASH_H_

#include "network_handler.h"

class get_dump_block_by_hash : public base_network_handler<get_dump_block_by_hash> {
public:
    typedef base_network_handler<get_dump_block_by_hash> base;
    
    get_dump_block_by_hash(http_session_ptr session): base(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_dump_block_by_hash() override {}
    
    virtual bool prepare_params() override;
    
};

#endif // GET_DUMP_BLOCK_BY_HASH_H_
