#pragma once

#include <string>
#include "base_handler.h"
#include "../http_json_rpc_request.h"

template <class T>
class base_network_handler : public base_handler<T>
{
public:
    base_network_handler(const std::string& host, http_session_ptr session): base_handler<T>(session)
    {
        m_request = std::make_shared<http_json_rpc_request>(host, session->get_io_context());
    }
    virtual ~base_network_handler() override { }

    virtual void execute() override;

protected:
    bool m_async_execute = {true};
    std::shared_ptr<http_json_rpc_request> m_request;
};

#define DECL_NETWORK_HANDLER(cl)\
    class cl : public base_network_handler<cl>\
    {\
    public:\
        typedef base_network_handler<cl> base;\
        cl(http_session_ptr session): base(settings::server::tor, session) {}\
        virtual ~cl() override {}\
        virtual bool prepare_params() override;\
    };


template <class T>
class base_send_tx_handler : public base_network_handler<T>
{
public:
    base_send_tx_handler(http_session_ptr session):
        base_network_handler<T>(settings::server::proxy, session) {}

    virtual ~base_send_tx_handler() override { }

    virtual bool prepare_params() override;

protected:
    virtual bool get_nonce(mh_count_t& result) = 0;

protected:
    std::string m_address;
};
