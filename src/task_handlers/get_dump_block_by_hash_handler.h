#ifndef __GET_DUMP_BLOCK_BY_HASH_H__
#define __GET_DUMP_BLOCK_BY_HASH_H__

#include "get_dump_block_base.h"

class get_dump_block_by_hash : public get_dump_block_base
{
public:
    get_dump_block_by_hash(session_context_ptr ctx);
    virtual ~get_dump_block_by_hash() override {}

protected:
    virtual void set_block_id(const rapidjson::Value* params);
};

#endif // __GET_DUMP_BLOCK_BY_HASH_H__
