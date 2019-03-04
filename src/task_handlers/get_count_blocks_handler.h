#ifndef __GET_COUNT_BLOCKS_HANDLER_H__
#define __GET_COUNT_BLOCKS_HANDLER_H__

#include "network_handler.h"
#include "settings/settings.h"

class get_count_blocks_handler : public base_network_handler
{
public:
    get_count_blocks_handler(http_session_ptr session)
        : base_network_handler(settings::server::tor, session) {
        m_duration.set_message(__func__);
    }

    virtual ~get_count_blocks_handler() override {}

protected:
    virtual bool prepare_params() override;
};

#endif // __GET_COUNT_BLOCKS_HANDLER_H__
