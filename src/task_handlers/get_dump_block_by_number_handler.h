#ifndef __GET_DUMP_BLOCK_BY_NUMBER_H__
#define __GET_DUMP_BLOCK_BY_NUMBER_H__

#include "get_dump_block_base.h"

class get_dump_block_by_number : public get_dump_block_base
{
public:
    get_dump_block_by_number(session_context_ptr ctx);
    virtual ~get_dump_block_by_number() override {}

private:
    virtual void set_block_id(const rapidjson::Value* params) override;
};

#endif // __GET_DUMP_BLOCK_BY_NUMBER_H__
