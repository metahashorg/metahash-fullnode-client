#ifndef __GET_DUMP_BLOCK_BY_HASH_H__
#define __GET_DUMP_BLOCK_BY_HASH_H__

#include "network_handler.h"
#include "settings/settings.h"

class get_dump_block_by_hash : public base_network_handler
{
public:
    get_dump_block_by_hash(http_session_ptr session)
        : base_network_handler(settings::server::tor, session) {
        m_duration.set_message(__func__);
    }
    virtual ~get_dump_block_by_hash() override {}

protected:
    virtual bool prepare_params() override;
};

#endif // __GET_DUMP_BLOCK_BY_HASH_H__
