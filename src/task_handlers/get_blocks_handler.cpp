#include "get_blocks_handler.h"
#include "settings/settings.h"

get_blocks_handler::get_blocks_handler(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx) {
    m_duration.set_message(__func__);
    m_name = __func__;
}

bool get_blocks_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        mh_count_t countBlocks(0);
        CHK_PRM(m_reader.get_value(*params, "countBlocks", countBlocks), "'countBlocks' field not found")
        m_writer.add_param("countBlocks", countBlocks);

        mh_count_t beginBlock(0);
        CHK_PRM(m_reader.get_value(*params, "beginBlock", beginBlock), "'beginBlock' field not found")
        m_writer.add_param("beginBlock", beginBlock);

        std::string type;
        if (m_reader.get_value(*params, "type", type)) {
            m_writer.add_param("type", type);
        }
        std::string direction;
        if (m_reader.get_value(*params, "direction", direction)) {
            m_writer.add_param("direction", direction);
        }
//        bool pretty(false);
//        if (m_reader.get_value(m_reader.get_doc(), "pretty", pretty) && pretty) {
//            m_writer.add("pretty", pretty);
//        }

        return true;
    }
    END_TRY(return false)
}
