#include "addr_validate_handler.h"

#include "utils.h"

bool addr_validate_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "address", m_address)  &&!m_address.empty(), "address field not found")

        return true;
    }
    END_TRY_RET(false)
}

void addr_validate_handler::execute()
{
    BGN_TRY
    {
        CHK_PRM(utils::validate_address(m_address), "address not valid")
        m_writer.add_result("validate", "ok");
    }
    END_TRY
}
