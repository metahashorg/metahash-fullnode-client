#ifndef __SEND_TX_HANDLER_H__
#define __SEND_TX_HANDLER_H__

#include "create_tx_base_handler.h"

class send_tx_handler : public create_tx_base_handler
{
public:
    send_tx_handler(http_session_ptr session)
        : create_tx_base_handler(session)
        , m_type(0) {}
    virtual ~send_tx_handler() override {}

protected:
    virtual bool prepare_params() override;

    void on_get_balance(http_json_rpc_request_ptr request);

    void process_response(json_rpc_reader &reader) override;

    bool check_send_params();
    void get_type();

protected:
    std::string m_transaction;
    std::string m_pubkey;
    std::string m_sign;
    mh_count_t m_type;
};

#endif // __SEND_TX_HANDLER_H__
