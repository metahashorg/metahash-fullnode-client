#pragma once

#include <string>
#include "base_handler.h"
#include "../http_json_rpc_request.h"

class base_network_handler : public base_handler, public std::enable_shared_from_this<base_network_handler>
{
public:
    base_network_handler(const std::string& host, http_session_ptr session): base_handler(session)
    {
        m_request = std::make_shared<http_json_rpc_request>(host, session->get_io_context());
    }
    virtual ~base_network_handler() override { }

    virtual void execute() override;

protected:

    // async callback
    void on_complete(json_rpc_id id, http_json_rpc_request_ptr req, http_session_ptr session);

protected:
    
    virtual void processResponse(json_rpc_id id, http_json_rpc_request_ptr req);
    
protected:
    bool                        m_async_execute = {true};
    http_json_rpc_request_ptr   m_request;
};
