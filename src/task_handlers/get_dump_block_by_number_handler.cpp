#include "get_dump_block_by_number.h"

bool get_dump_block_by_number::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found");

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        mh_count_t number(0);
        auto jValue = this->m_reader.get("number", *params);
        CHK_PRM(jValue, "number field not found")

        std::string tmp;
        CHK_PRM(json_utils::val2str(jValue, tmp), "number field incorrect format")
        number = std::stoull(tmp);
        m_writer.add_param("number", number);

        return true;
    }
    END_TRY_RET(false)
}
