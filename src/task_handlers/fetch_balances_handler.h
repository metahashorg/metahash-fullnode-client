#ifndef __FETCH_BALANCES_HANDLER_H__
#define __FETCH_BALANCES_HANDLER_H__

#include "network_handler.h"

class fetch_balances_handler : public base_network_handler
{
public:
    fetch_balances_handler(session_context_ptr ctx);
    virtual ~fetch_balances_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

private:
    std::vector<std::string> m_addrs;
};

#endif // __FETCH_BALANCES_HANDLER_H__
