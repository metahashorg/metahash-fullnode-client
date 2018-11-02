#ifndef FETCH_HISTORY_HANDLER_SYNC_H_
#define FETCH_HISTORY_HANDLER_SYNC_H_

#include "sync_handler.h"

class fetch_history_handler_sync : public base_sync_handler<fetch_history_handler_sync> {
public:
    typedef base_sync_handler<fetch_history_handler_sync> base;
    
    fetch_history_handler_sync(http_session_ptr session): base(settings::server::tor, session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~fetch_history_handler_sync() override {}
    
    virtual bool prepare_params() override;
    
    virtual void executeImpl() override;
    
private:
    
    std::string address;
};

#endif // FETCH_HISTORY_HANDLER_SYNC_H_
