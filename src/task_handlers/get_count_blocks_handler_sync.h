#ifndef GET_COUNT_BLOCKS_HANDLER_SYNC_H_
#define GET_COUNT_BLOCKS_HANDLER_SYNC_H_

#include "sync_handler.h"

class get_count_blocks_handler_sync : public base_sync_handler, public Perform<get_count_blocks_handler_sync> {
public:
    
    get_count_blocks_handler_sync(http_session_ptr session): base_sync_handler(session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~get_count_blocks_handler_sync() override {}
    
    virtual bool prepare_params() override;
    
    virtual void executeImpl() override;
    
private:
};

#endif // GET_COUNT_BLOCKS_HANDLER_SYNC_H_
