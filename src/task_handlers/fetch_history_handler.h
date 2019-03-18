#ifndef __FETCH_HISTORY_HANDLER_H__
#define __FETCH_HISTORY_HANDLER_H__

#include "network_handler.h"

class fetch_history_handler : public base_network_handler
{
public:
    fetch_history_handler(http_session_ptr session);
    virtual ~fetch_history_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

private:
    std::string m_addr;
    mh_count_t m_countTxs;
    mh_count_t m_beginTx;
};

#endif // __FETCH_HISTORY_HANDLER_H__
