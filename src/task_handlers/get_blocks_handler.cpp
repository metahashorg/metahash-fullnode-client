#include "task_handlers.h"

// get_blocks_handler
bool get_blocks_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        mh_count_t countBlocks(0);
        auto jValue = this->m_reader.get("countBlocks", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "countBlocks field incorrect format")
            countBlocks = std::stoull(tmp);
            m_writer.add_param("countBlocks", countBlocks);
        }

        mh_count_t beginBlock(0);
        jValue = this->m_reader.get("beginBlock", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "beginBlock field incorrect format")
            beginBlock = std::stoull(tmp);
            m_writer.add_param("beginBlock", beginBlock);
        }

        return true;
    }
    END_TRY_RET(false)
}
