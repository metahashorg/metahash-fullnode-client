#include "get_blocks_handler.h"

bool get_blocks_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        auto &jsonParams = *params;
        
        int64_t countBlocks(0);
        if (jsonParams.HasMember("countBlocks") && jsonParams["countBlocks"].IsInt64()) {
            countBlocks = jsonParams["countBlocks"].GetInt64();
            m_writer.add_param("countBlocks", countBlocks);
        }
        
        int64_t beginBlock(0);
        if (jsonParams.HasMember("beginBlock") && jsonParams["beginBlock"].IsInt64()) {
            beginBlock = jsonParams["beginBlock"].GetInt64();
            m_writer.add_param("beginBlock", beginBlock);
        }

        return true;
    }
    END_TRY_RET(false)
}
