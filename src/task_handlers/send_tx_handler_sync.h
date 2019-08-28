#ifndef __SEND_TX_HANDLER_SYNC_H__
#define __SEND_TX_HANDLER_SYNC_H__

#include "create_tx_base_handler.h"

class send_tx_handler_sync : public create_tx_base_handler
{
public:
    send_tx_handler_sync(session_context_ptr ctx)
        : create_tx_base_handler(ctx) {
        m_name = __func__;
    }
    virtual ~send_tx_handler_sync() override {}

protected:
    virtual bool prepare_params() override;
    virtual void process_response(json_rpc_reader &reader) override;

    void on_get_balance();
};

#endif // __SEND_TX_HANDLER_SYNC_H__
