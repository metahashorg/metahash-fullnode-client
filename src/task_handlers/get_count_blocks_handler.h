#ifndef __GET_COUNT_BLOCKS_HANDLER_H__
#define __GET_COUNT_BLOCKS_HANDLER_H__

#include "network_handler.h"

class get_count_blocks_handler : public base_network_handler
{
public:
    get_count_blocks_handler(http_session_ptr session);
    virtual ~get_count_blocks_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
};

#endif // __GET_COUNT_BLOCKS_HANDLER_H__
