#include "fetch_history_handler.h"

bool fetch_history_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        std::string addr;
        CHK_PRM(m_reader.get_value(*params, "address", addr), "address field not found")
        CHK_PRM(!addr.empty(), "address is empty")
        CHK_PRM(addr.compare(0, 2, "0x") == 0, "address field incorrect format")

        m_writer.add_param("address", addr.c_str());

        mh_count_t countTxs(0);
        auto jValue = this->m_reader.get("countTxs", *params);
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
