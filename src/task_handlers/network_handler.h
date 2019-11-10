#ifndef __NETWORK_HANDLER_H__
#define __NETWORK_HANDLER_H__

#include <string>
#include "base_handler.h"
#include "http_json_rpc_request_ptr.h"

class base_network_handler : public base_handler
{
public:
    base_network_handler(std::string&& host, session_context_ptr session_ctx);
    virtual ~base_network_handler() override;

    virtual void execute() override;

protected:
    // async callback
    void on_complete();

    virtual void process_response(json_rpc_reader &reader);
    void send_response();

protected:
    http_json_rpc_request_ptr m_request;
};

#endif // __NETWORK_HANDLER_H__
