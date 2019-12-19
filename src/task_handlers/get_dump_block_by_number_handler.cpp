#include "get_dump_block_by_number_handler.h"
#include "settings/settings.h"

get_dump_block_by_number::get_dump_block_by_number(session_context_ptr ctx)
    : get_dump_block_base(ctx)
{
    m_duration.set_message(__func__);
    m_name = __func__;
}

void get_dump_block_by_number::set_block_id(const rapidjson::Value* params)
{
    std::size_t number;
    CHK_PRM(m_reader.get_value(*params, "number", number), "number field not found")
    if (!settings::system::useLocalDatabase) {
        m_writer.add_param("number", number);
    }
    m_block_id = number;
}
