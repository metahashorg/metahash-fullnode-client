#pragma once

#include <string>

#include "base_handler.h"
#include "http_json_rpc_request_ptr.h"

class base_network_handler : public base_handler, public std::enable_shared_from_this<base_network_handler>
{
public:
    base_network_handler(const std::string& host, http_session_ptr session);
    
    virtual ~base_network_handler() override { }

    virtual void execute() override;

protected:

    // async callback
    void on_complete(json_rpc_id id, http_json_rpc_request_ptr req, http_session_ptr session);

protected:
    
    virtual void processResponse(json_rpc_id id, json_rpc_reader &reader);
    
protected:
    bool                        m_async_execute = {true};
    http_json_rpc_request_ptr   m_request;
};
