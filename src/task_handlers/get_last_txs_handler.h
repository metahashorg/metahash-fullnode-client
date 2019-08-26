#ifndef __GET_LAST_TXS_HANDLER_H__
#define __GET_LAST_TXS_HANDLER_H__

#include "network_handler.h"

class get_last_txs_handler : public base_network_handler
{
public:
    get_last_txs_handler(session_context_ptr ctx);
    virtual ~get_last_txs_handler() override {}

protected:
    virtual bool prepare_params() override;
};

#endif // __GET_LAST_TXS_HANDLER_H__
