#pragma once
#include "network_handler.h"

class send_tx_handler : public base_send_tx_handler<send_tx_handler>
{
public:
    typedef base_send_tx_handler<send_tx_handler> base;

    send_tx_handler(http_session_ptr session): base(session) {}
    virtual ~send_tx_handler() override {}

protected:
    virtual bool get_nonce(mh_count_t& result) override;
};
