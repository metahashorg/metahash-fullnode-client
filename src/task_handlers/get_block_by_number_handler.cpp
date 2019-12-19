#include "get_block_by_number_handler.h"
#include "settings/settings.h"

get_block_by_number_handler::get_block_by_number_handler(session_context_ptr ctx)
    : get_block_base(ctx)
{
    m_duration.set_message(__func__);
    m_name = __func__;
}

void get_block_by_number_handler::set_block_id(const rapidjson::Value* params)
{
    size_t number = 0;
    CHK_PRM(m_reader.get_value(*params, "number", number), "number field not found")
    if (!settings::system::useLocalDatabase) {
        m_writer.add_param("number", number);
    }
    m_block_id = number;
}


