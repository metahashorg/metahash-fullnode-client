#ifndef __GET_BLOCK_BY_HASH_HANDLER_H__
#define __GET_BLOCK_BY_HASH_HANDLER_H__

#include "network_handler.h"
#include "settings/settings.h"

class get_block_by_hash_handler : public base_network_handler
{
public:
    get_block_by_hash_handler(http_session_ptr session)
        : base_network_handler(settings::server::tor, session) {
        m_duration.set_message(__func__);
    }
    virtual ~get_block_by_hash_handler() override {}

protected:
    virtual bool prepare_params() override;
};

#endif // __GET_BLOCK_BY_HASH_HANDLER_H__
