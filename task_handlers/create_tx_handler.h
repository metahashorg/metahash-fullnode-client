#pragma once
#include "network_handler.h"

class create_tx_handler : public base_send_tx_handler<create_tx_handler>
{
public:
    typedef base_send_tx_handler<create_tx_handler> base;

    create_tx_handler(http_session_ptr session): base(session) {}
    virtual ~create_tx_handler() override {}

protected:
    virtual bool get_nonce(mh_count_t& result) override;
};
