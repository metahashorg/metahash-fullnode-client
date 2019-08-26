#ifndef __FETCH_TRANSACTION_HANDLER_H__
#define __FETCH_TRANSACTION_HANDLER_H__

#include "base_handler.h"

class fetch_transaction_handler : public base_handler
{
public:
    fetch_transaction_handler(session_context_ptr ctx);
    virtual ~fetch_transaction_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

private:
    std::string m_addr;
    std::string m_data;
};

#endif // __FETCH_TRANSACTION_HANDLER_H__
