#ifndef __GENERATE_HANDLER_H__
#define __GENERATE_HANDLER_H__

#include "base_handler.h"

class generate_handler : public base_handler
{
public:
    generate_handler(http_session_ptr session)
        : base_handler(session) {
        m_duration.set_message(__func__);
    }
    
    virtual ~generate_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
};

#endif // __GENERATE_HANDLER_H__
