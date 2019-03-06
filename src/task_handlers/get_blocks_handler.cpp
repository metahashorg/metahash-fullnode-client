#include "get_blocks_handler.h"

bool get_blocks_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        mh_count_t countBlocks(0);
        if (m_reader.get_value(*params, "countBlocks", countBlocks)) {
            m_writer.add_param("countBlocks", countBlocks);
        }

        mh_count_t beginBlock(0);
        if (m_reader.get_value(*params, "beginBlock", beginBlock)) {
            m_writer.add_param("beginBlock", beginBlock);
        }

        return true;
    }
    END_TRY_RET(false)
}
