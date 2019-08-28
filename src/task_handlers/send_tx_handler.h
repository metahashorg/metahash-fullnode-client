#ifndef __SEND_TX_HANDLER_H__
#define __SEND_TX_HANDLER_H__

#include "create_tx_base_handler.h"

class send_tx_handler : public create_tx_base_handler
{
public:
    send_tx_handler(session_context_ptr ctx)
        : create_tx_base_handler(ctx) {
        m_duration.set_message(__func__);
        m_name = __func__;
    }
    virtual ~send_tx_handler() override {}

protected:
    virtual bool prepare_params() override;

    void on_get_balance(http_json_rpc_request_ptr request);

    void process_response(json_rpc_reader &reader) override;

    int check_params();
    int check_params_1();
    int check_params_2();
};

#endif // __SEND_TX_HANDLER_H__
