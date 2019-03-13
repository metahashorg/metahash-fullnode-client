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

    inline void set_async(bool value) {
        m_async_execute = value;
    }

protected:
    // async callback
    void on_complete();

protected:
    virtual void process_response(json_rpc_reader &reader);
    void send_response();

protected:
    bool m_async_execute = {true};
    http_json_rpc_request_ptr m_request;
};

#endif // __NETWORK_HANDLER_H__
