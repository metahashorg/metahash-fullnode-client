#ifndef GET_DUMP_BLOCK_BY_HASH_HANDLER_SYNC_H_
#define GET_DUMP_BLOCK_BY_HASH_HANDLER_SYNC_H_

#include "sync_handler.h"

class get_dump_block_by_hash_handler_sync : public base_sync_handler
{
public:
    get_dump_block_by_hash_handler_sync(http_session_ptr session)
        : base_sync_handler(session) {
        m_duration.set_message(__func__);
    }
    virtual ~get_dump_block_by_hash_handler_sync() override {}

    virtual void executeImpl() override;

protected:
    virtual bool prepare_params() override;

private:
    std::string hash;
    size_t fromByte;
    size_t toByte;
    bool isHex;
};

#endif // GET_DUMP_BLOCK_BY_HASH_HANDLER_SYNC_H_
