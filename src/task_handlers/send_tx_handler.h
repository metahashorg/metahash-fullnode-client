#pragma once
#include "create_tx_base_handler.h"

class send_tx_handler : public create_tx_base_handler, public Perform<send_tx_handler>/*, std::enable_shared_from_this<send_tx_handler>*/
{
public:

    send_tx_handler(http_session_ptr session): create_tx_base_handler(session) {}
    virtual ~send_tx_handler() override {}

    virtual bool prepare_params() override;
    
protected:
    
    void on_get_balance(http_json_rpc_request_ptr request, json_rpc_id id, std::shared_ptr<base_network_handler> tmp);
    
protected:
    
    void processResponse(json_rpc_id id, http_json_rpc_request_ptr req) override;
};
