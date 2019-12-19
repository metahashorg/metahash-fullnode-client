#include "get_dump_block_by_hash_handler.h"
#include "settings/settings.h"

get_dump_block_by_hash::get_dump_block_by_hash(session_context_ptr ctx)
    : get_dump_block_base(ctx)
{
    m_duration.set_message(__func__);
    m_name = __func__;
}

void get_dump_block_by_hash::set_block_id(const rapidjson::Value* params)
{
    std::string hash;
    CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found");
    if (!settings::system::useLocalDatabase) {
        m_writer.add_param("hash", hash);
    }
    m_block_id = std::move(hash);
}
