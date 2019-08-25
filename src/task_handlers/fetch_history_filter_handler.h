#ifndef __FETCH_HISTORY_FILTER_HANDLER_H__
#define __FETCH_HISTORY_FILTER_HANDLER_H__

#include "network_handler.h"

class fetch_history_filter_handler : public base_network_handler
{
    static const std::vector<const char*> key_str;

public:
    fetch_history_filter_handler(http_session_ptr session);
    virtual ~fetch_history_filter_handler() override {}

protected:
    virtual bool prepare_params() override;

private:
    std::string m_addr;
};

#endif // __FETCH_HISTORY_FILTER_HANDLER_H__
