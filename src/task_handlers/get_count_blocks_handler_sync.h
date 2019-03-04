#ifndef GET_COUNT_BLOCKS_HANDLER_SYNC_H_
#define GET_COUNT_BLOCKS_HANDLER_SYNC_H_

#include "sync_handler.h"

class get_count_blocks_handler_sync : public base_sync_handler
{
public:
    get_count_blocks_handler_sync(http_session_ptr session): base_sync_handler(session) {
        m_duration.set_message(__func__);
    }
    virtual ~get_count_blocks_handler_sync() override {}

protected:
    virtual bool prepare_params() override;
    virtual void executeImpl() override;
};

#endif // GET_COUNT_BLOCKS_HANDLER_SYNC_H_
