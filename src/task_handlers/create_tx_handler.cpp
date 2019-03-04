#include "create_tx_handler.h"

// create_tx_handler
bool create_tx_handler::prepare_params()
{
    BGN_TRY
    {
        if (!check_params())
            return false;

        auto params = m_reader.get_params();
        auto jValue = m_reader.get("nonce", *params);

        CHK_PRM(jValue, "nonce field not found")
        std::string tmp;
        CHK_PRM(json_utils::val2str(jValue, tmp), "nonce field has incorrect format")
        this->m_nonce = std::stoull(tmp);

        if (!build_request())
            return false;

        return true;
    }
    END_TRY_RET(false)
}

void create_tx_handler::execute()
{
    // do nothing, just prepare
    return;
}
