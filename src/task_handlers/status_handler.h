#ifndef STATUS_HANDLER_H_
#define STATUS_HANDLER_H_

#include "base_handler.h"

class status_handler : public base_handler, public Perform<status_handler> {
public:
    status_handler(http_session_ptr session): base_handler(session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~status_handler() override {}
    
    virtual bool prepare_params() override;
    
    virtual void execute() override;
};

#endif // STATUS_HANDLER_H_
