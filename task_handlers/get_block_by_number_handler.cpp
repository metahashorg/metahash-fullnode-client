#include "task_handlers.h"

// get_block_by_number_handler
bool get_block_by_number_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        mh_count_t number(0);
        CHK_PRM(m_reader.get_value(*params, "number", number), "number field not found")

        m_writer.add_param("number", number);

        uint32_t type(0);
        if (m_reader.get_value(*params, "type", type))
            m_writer.add_param("type", type);

        mh_count_t countTxs(0);
        if (m_reader.get_value(*params, "countTxs", countTxs))
            m_writer.add_param("countTxs", countTxs);

        mh_count_t beginTx(0);
        if (m_reader.get_value(*params, "beginTx", beginTx))
            m_writer.add_param("beginTx", beginTx);

        return true;
    }
    END_TRY_RET(false)
}
