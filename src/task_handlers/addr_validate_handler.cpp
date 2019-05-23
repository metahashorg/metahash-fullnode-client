#include "addr_validate_handler.h"

#include "cpplib_open_ssl_decor/crypto.h"

bool addr_validate_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "address", m_address)  &&!m_address.empty(), "address field not found")
        CHK_PRM(m_address.compare(0, 2, "0x") == 0, "address field must be in hex format")

        return true;
    }
    END_TRY_RET(false)
}

void addr_validate_handler::execute()
{
    BGN_TRY
    {
        CHK_PRM(m_address.compare(2, 2, "00") == 0, "address not valid")
        CHK_PRM(m_address.size() == 26*2, "address not valid")
        CHK_PRM(CRYPTO_check_address(m_address), "address not valid");
        m_writer.add_result("validate", "ok");
    }
    END_TRY
}
