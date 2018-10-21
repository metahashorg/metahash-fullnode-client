#include "task_handlers.h"

// get_dump_block_by_number
bool get_dump_block_by_number::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found");

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        mh_count_t number(0);
        CHK_PRM(m_reader.get_value(*params, "number", number), "number field not found")

        m_writer.add_param("number", number);

        return true;
    }
    END_TRY_RET(false)
}
