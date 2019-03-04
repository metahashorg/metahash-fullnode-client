#ifndef __SEND_TX_HANDLER_H__
#define __SEND_TX_HANDLER_H__

#include "create_tx_base_handler.h"

class send_tx_handler : public create_tx_base_handler
{
public:
    send_tx_handler(http_session_ptr session)
        : create_tx_base_handler(session) {}
    virtual ~send_tx_handler() override {}

protected:
    virtual bool prepare_params() override;

    void on_get_balance(http_json_rpc_request_ptr request, json_rpc_id id);

protected:
    void processResponse(json_rpc_id id, json_rpc_reader &reader) override;
};

#endif // __SEND_TX_HANDLER_H__
