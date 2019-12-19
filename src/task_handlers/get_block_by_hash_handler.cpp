#include "get_block_by_hash_handler.h"
#include "settings/settings.h"

get_block_by_hash_handler::get_block_by_hash_handler(session_context_ptr ctx)
    : get_block_base(ctx)
{
    m_duration.set_message(__func__);
    m_name = __func__;
}

void get_block_by_hash_handler::set_block_id(const rapidjson::Value* params)
{
    std::string hash;
    CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
    CHK_PRM(!hash.empty(), "hash is empty")

    if (!settings::system::useLocalDatabase) {
        m_writer.add_param("hash", hash);
    }
    m_block_id = std::move(hash);
}
