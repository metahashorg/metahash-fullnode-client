#ifndef __GENERATE_HANDLER_H__
#define __GENERATE_HANDLER_H__

#include "base_handler.h"

class generate_handler : public base_handler
{
public:
    generate_handler(session_context_ptr ctx)
        : base_handler(ctx) {
        m_duration.set_message(__func__);
        m_name = __func__;
    }
    
    virtual ~generate_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
};

#endif // __GENERATE_HANDLER_H__
