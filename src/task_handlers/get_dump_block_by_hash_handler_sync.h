#ifndef GET_DUMP_BLOCK_BY_HASH_HANDLER_SYNC_H_
#define GET_DUMP_BLOCK_BY_HASH_HANDLER_SYNC_H_

#include "sync_handler.h"

class get_dump_block_by_hash_handler_sync : public base_sync_handler
{
public:
    get_dump_block_by_hash_handler_sync(session_context_ptr ctx)
        : base_sync_handler(ctx) {
        m_duration.set_message(__func__);
        m_name = __func__;
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
