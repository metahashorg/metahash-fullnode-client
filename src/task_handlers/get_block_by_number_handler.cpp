#include "get_block_by_number_handler.h"

bool get_block_by_number_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        auto jValue = this->m_reader.get("number", *params);
        CHK_PRM(jValue, "number field not found")

        std::string tmp;
        CHK_PRM(json_utils::val2str(jValue, tmp), "number field incorrect format")
        mh_count_t number = std::stoull(tmp);
        m_writer.add_param("number", number);

        uint32_t type(0);
        jValue = this->m_reader.get("type", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "type field incorrect format")
            type = static_cast<uint32_t>(std::stoi(tmp));
            m_writer.add_param("type", type);
        }

        mh_count_t countTxs(0);
        jValue = this->m_reader.get("countTxs", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "countTxs field incorrect format")
            countTxs = std::stoull(tmp);
            m_writer.add_param("countTxs", countTxs);
        }

        mh_count_t beginTx(0);
        jValue = this->m_reader.get("beginTx", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "beginTx field incorrect format")
            beginTx = std::stoull(tmp);
            m_writer.add_param("beginTx", beginTx);
        }

        return true;
    }
    END_TRY_RET(false)
}
