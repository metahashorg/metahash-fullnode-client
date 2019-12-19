#ifndef __GET_BLOCK_BY_HASH_HANDLER_H__
#define __GET_BLOCK_BY_HASH_HANDLER_H__

#include "get_block_base.h"

class get_block_by_hash_handler : public get_block_base
{
public:
    get_block_by_hash_handler(session_context_ptr ctx);
    virtual ~get_block_by_hash_handler() override {}

protected:
    virtual void set_block_id(const rapidjson::Value* params) override;
};

#endif // __GET_BLOCK_BY_HASH_HANDLER_H__
