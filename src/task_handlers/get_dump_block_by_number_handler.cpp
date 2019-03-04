#include "get_dump_block_by_number.h"

bool get_dump_block_by_number::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found");

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        int64_t number(0);
        auto &jsonParams = *params;
        CHK_PRM(jsonParams.HasMember("number") && jsonParams["number"].IsInt64(), "number field not found")
        number = jsonParams["number"].GetInt64();
        m_writer.add_param("number", number);

        m_writer.add_param("isHex", true);
        
        return true;
    }
    END_TRY_RET(false)
}
