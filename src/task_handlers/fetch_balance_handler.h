#ifndef __FETCH_BALANCE_HANDLER_H__
#define __FETCH_BALANCE_HANDLER_H__

#include "network_handler.h"

class fetch_balance_handler : public base_network_handler
{
public:
    fetch_balance_handler(session_context_ptr ctx);
    virtual ~fetch_balance_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

private:
    std::string m_addr;
};

#endif // __FETCH_BALANCE_HANDLER_H__
