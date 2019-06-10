#include "create_tx_handler.h"
#include "utils.h"
#include "common/convertStrings.h"
#include "cpplib_open_ssl_decor/crypto.h"

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
bool create_tx_handler_v2::build_request()
{
    BGN_TRY
    {
        if (m_fee == 0) {
            m_fee = m_data.size();
        }
        m_data = common::toHex(m_data.begin(), m_data.end());
        std::string sign;
        std::string transaction;
        CHK_PRM(utils::gen_sign(transaction, sign, m_keys.prv_key, "xUUUux", m_to.c_str(), m_value, m_fee, m_nonce, m_data.size() / 2, m_data.c_str()), "failed on gen sign")
        std::string hash;
        CHK_PRM(utils::make_tx(hash, "xuxux", transaction.c_str(), sign.size() / 2, sign.c_str(), m_keys.pub_key.size() / 2, m_keys.pub_key.c_str()), "failed on make tx")

        m_writer.reset();
        //m_writer.add_result("method", "mhc_send");
        m_writer.add_result("transaction", transaction.c_str());
        m_writer.add_result("to", m_to.c_str());
        m_writer.add_result("value", std::to_string(m_value));
        m_writer.add_result("fee", !m_fee ? "" : std::to_string(m_fee));
        m_writer.add_result("nonce", std::to_string(m_nonce));
        m_writer.add_result("data", m_data.c_str());
        m_writer.add_result("pubkey", m_keys.pub_key);
        m_writer.add_result("sign", sign);
        m_writer.add_result("hash", hash);

        return true;
    }
    END_TRY_RET(false)
}
