#pragma once
#include "create_tx_base_handler.h"

class send_tx_handler : public create_tx_base_handler<send_tx_handler>/*, std::enable_shared_from_this<send_tx_handler>*/
{
public:
    typedef create_tx_base_handler<send_tx_handler> base;

    send_tx_handler(http_session_ptr session): base(session) {}
    virtual ~send_tx_handler() override {}

    virtual bool prepare_params() override;

protected:
    void on_get_balance(http_json_rpc_request_ptr request, json_rpc_id id);
};
