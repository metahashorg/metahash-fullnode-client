#include "fetch_balance_handler.h"

bool fetch_balance_handler::prepare_params()
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

        return true;
    }
    END_TRY_RET(false)
}
