#include "create_tx_handler.h"
#include "utils.h"
#include "common/convertStrings.h"
//#include "cpplib_open_ssl_decor/crypto.h"

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
        m_nonce = std::stoull(tmp);

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


// create_tx_handler_v2
bool create_tx_handler_v2::build_request(bool)
{
    return create_tx_handler::build_request(true);
}

void create_tx_handler_v2::make_json()
{
    BGN_TRY
    {
        m_writer.reset();
        //m_writer.add_result("method", "mhc_send");
        m_writer.add_result("transaction", m_transaction.c_str());
        m_writer.add_result("to", m_to.c_str());
        m_writer.add_result("value", std::to_string(m_value));
        m_writer.add_result("fee", !m_fee ? "" : std::to_string(m_fee));
        m_writer.add_result("nonce", std::to_string(m_nonce));
        m_writer.add_result("data", m_data.c_str());
        m_writer.add_result("pubkey", m_keys.pub_key);
        m_writer.add_result("sign", m_sign);
        m_writer.add_result("hash", m_hash);
    }
    END_TRY
}
