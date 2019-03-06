#ifndef __NETWORK_HANDLER_H__
#define __NETWORK_HANDLER_H__

#include <string>
#include "base_handler.h"
#include "http_json_rpc_request_ptr.h"

class base_network_handler : public base_handler
{
public:
    base_network_handler(const std::string& host, http_session_ptr session);
    virtual ~base_network_handler() override;

    virtual void execute() override;

protected:

    // async callback
    void on_complete();

protected:
    virtual void processResponse(json_rpc_reader &reader);

protected:
    bool m_async_execute = {true};
    http_json_rpc_request_ptr m_request;
};

#endif // __NETWORK_HANDLER_H__
