#ifndef GENERATE_HANDLER_H_
#define GENERATE_HANDLER_H_

#include "base_handler.h"

class generate_handler : public base_handler, public Perform<generate_handler> {
public:
    generate_handler(http_session_ptr session): base_handler(session) {
        std::stringstream ss;
        ss << __FUNCTION__;
        m_duration.set_message(ss.str());
    }
    
    virtual ~generate_handler() override {}
    
    virtual bool prepare_params() override;
    
    virtual void execute() override;
};

#endif // GENERATE_HANDLER_H_
