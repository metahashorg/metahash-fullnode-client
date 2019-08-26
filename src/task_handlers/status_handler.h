#ifndef STATUS_HANDLER_H_
#define STATUS_HANDLER_H_

#include "base_handler.h"

class status_handler : public base_handler
{
    enum class cmd {
        general,
        keys
    };

public:
    status_handler(session_context_ptr ctx): base_handler(ctx) {
        m_duration.set_message(__func__);
    }
    virtual ~status_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

protected:
    cmd m_cmd = {cmd::general};
};

#endif // STATUS_HANDLER_H_
