#ifndef __ADDR_VALIDATE_HANDLER_H__
#define __ADDR_VALIDATE_HANDLER_H__

#include "base_handler.h"

class addr_validate_handler : public base_handler
{
public:
    addr_validate_handler(session_context_ptr ctx)
        : base_handler(ctx) {
        m_duration.set_message(__func__);
    }
    
    virtual ~addr_validate_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

protected:
    std::string m_address;
};

#endif // __ADDR_VALIDATE_HANDLER_H__
