#ifndef __GET_TX_HANDLER_H__
#define __GET_TX_HANDLER_H__

#include "network_handler.h"

class get_tx_handler : public base_network_handler
{
public:
    get_tx_handler(http_session_ptr session);
    virtual ~get_tx_handler() override {}

    virtual void execute() override;

protected:
    virtual bool prepare_params() override;

private:
    std::string m_hash;
};

#endif //

