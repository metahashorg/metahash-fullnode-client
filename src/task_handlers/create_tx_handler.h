#ifndef __CREATE_TX_HANDLER_H__
#define __CREATE_TX_HANDLER_H__

//#include "base_handler.h"
#include "create_tx_base_handler.h"

class create_tx_handler : public create_tx_base_handler
{
public:
    create_tx_handler(session_context_ptr ctx)
        : create_tx_base_handler(ctx) {
        m_duration.set_message(__func__);
        m_name = __func__;
    }
    virtual ~create_tx_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;
};


class create_tx_handler_v2 : public create_tx_handler
{
public:
    create_tx_handler_v2(session_context_ptr session)
        : create_tx_handler(session) {
        m_duration.set_message(__func__);
        m_name = __func__;
    }
    virtual ~create_tx_handler_v2() override {}

protected:
    virtual bool build_request(bool create_hash = false) override;
    virtual void make_json() override;
};

#endif // __CREATE_TX_HANDLER_H__
