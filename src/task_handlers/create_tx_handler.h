#pragma once

#include "create_tx_base_handler.h"

#include "base_handler.h"

class create_tx_handler : public create_tx_base_handler, public Perform<create_tx_handler>
{
public:
    create_tx_handler(http_session_ptr session): create_tx_base_handler(session) {}
    virtual ~create_tx_handler() override {}

    virtual void execute() override
    {
        // do nothing, just prepare
        return;
    }

    virtual bool prepare_params() override;
};
