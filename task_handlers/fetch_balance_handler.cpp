#include "task_handlers.h"

// fetch_balance_handler
bool fetch_balance_handler::prepare_params()
{
    CHK_PRM(m_id, "id field not found")

    auto params = m_reader.get_params();
    CHK_PRM(params, "params field not found")

    std::string addr;
    CHK_PRM(m_reader.get_value(*params, "address", addr) && !addr.empty(), "address field not found")

    m_writer.add_param("address", addr.c_str());

    return true;
}
