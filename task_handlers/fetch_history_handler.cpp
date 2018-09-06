#include "task_handlers.h"

// fetch_history_handler
bool fetch_history_handler::prepare_params()
{
    CHK_PRM(m_id, "id field not found")

    auto params = m_reader.get_params();
    CHK_PRM(params, "params field not found")

    std::string addr;
    CHK_PRM(m_reader.get_value(*params, "address", addr) && !addr.empty(), "address field not found")

    m_writer.add_param("address", addr.c_str());

    mh_count_t contTxs(0);
    if (m_reader.get_value(*params, "countTxs", contTxs))
        m_writer.add_param("countTxs", contTxs);

    mh_count_t beginTx(0);
    if (m_reader.get_value(*params, "beginTx", beginTx))
        m_writer.add_param("beginTx", beginTx);

    return true;
}
