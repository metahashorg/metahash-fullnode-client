#pragma once

#include "create_tx_base_handler.h"

class create_tx_handler : public create_tx_base_handler<create_tx_handler>
{
public:
    typedef create_tx_base_handler<create_tx_handler> base;

    create_tx_handler(http_session_ptr session): base(session) {}
    virtual ~create_tx_handler() override {}

    virtual void execute() override
    {
        // do nothing, just prepare
        return;
    }

    virtual bool prepare_params() override;
};
