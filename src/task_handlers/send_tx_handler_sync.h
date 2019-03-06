#ifndef __SEND_TX_HANDLER_SYNC_H__
#define __SEND_TX_HANDLER_SYNC_H__

#include "create_tx_base_handler.h"

class send_tx_handler_sync : public create_tx_base_handler
{
public:
    send_tx_handler_sync(http_session_ptr session): create_tx_base_handler(session) {}
    virtual ~send_tx_handler_sync() override {}

protected:
    virtual bool prepare_params() override;
    virtual void processResponse(json_rpc_reader &reader) override;

    void on_get_balance();
};

#endif // __SEND_TX_HANDLER_SYNC_H__
