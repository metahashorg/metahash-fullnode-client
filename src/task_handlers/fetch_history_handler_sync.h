#ifndef FETCH_HISTORY_HANDLER_SYNC_H_
#define FETCH_HISTORY_HANDLER_SYNC_H_

#include "sync_handler.h"

class fetch_history_handler_sync : public base_sync_handler
{
public:
    
    fetch_history_handler_sync(session_context_ptr ctx)
        : base_sync_handler(ctx) {
        m_duration.set_message(__func__);
    }
    virtual ~fetch_history_handler_sync() override {}

protected:
    virtual bool prepare_params() override;
    virtual void executeImpl() override;

private:
    std::string address;
    mh_count_t countTxs;
    mh_count_t beginTx;
};

#endif // FETCH_HISTORY_HANDLER_SYNC_H_
