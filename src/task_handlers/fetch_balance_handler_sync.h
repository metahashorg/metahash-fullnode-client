#ifndef FETCH_BALANCE_HANDLER_SYNC_H_
#define FETCH_BALANCE_HANDLER_SYNC_H_

#include "sync_handler.h"

class fetch_balance_handler_sync : public base_sync_handler
{
public:
    
    fetch_balance_handler_sync(session_context_ptr ctx): base_sync_handler(ctx) {
        m_duration.set_message(__func__);
        m_name = __func__;
    }
    virtual ~fetch_balance_handler_sync() override {}

protected:
    virtual bool prepare_params() override;
    virtual void executeImpl() override;

private:
    std::string address;
};

#endif // FETCH_BALANCE_HANDLER_SYNC_H_
