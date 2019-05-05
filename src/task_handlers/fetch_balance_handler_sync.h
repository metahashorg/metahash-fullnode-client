#ifndef FETCH_BALANCE_HANDLER_SYNC_H_
#define FETCH_BALANCE_HANDLER_SYNC_H_

#include "sync_handler.h"

class fetch_balance_handler_sync : public base_sync_handler
{
public:
    
    fetch_balance_handler_sync(http_session_ptr session): base_sync_handler(session) {
        m_duration.set_message(__func__);
    }
    virtual ~fetch_balance_handler_sync() override {}

protected:
    virtual bool prepare_params() override;
    virtual void executeImpl() override;

private:
    std::string address;
};

#endif // FETCH_BALANCE_HANDLER_SYNC_H_
