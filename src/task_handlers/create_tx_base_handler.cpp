#include "create_tx_base_handler.h"
#include "settings/settings.h"
#include "utils.h"
#include "common/convertStrings.h"

create_tx_base_handler::create_tx_base_handler(http_session_ptr session)
    : base_network_handler(settings::server::get_proxy(), session)
    , m_fee(0)
    , m_value(0)
    , m_nonce(0)
{}


bool create_tx_base_handler::check_params() {
    BGN_TRY {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "address", m_address)  &&!m_address.empty(), "address field not found")
        CHK_PRM(m_address.compare(0, 2, "0x") == 0, "address field must be in hex format")

        CHK_PRM(m_reader.get_value(*params, "to", m_to)  &&!m_to.empty(), "to field not found")
        CHK_PRM(m_to.compare(0, 2, "0x") == 0, "to field must be in hex format")

        auto jValue = m_reader.get("value", *params);
        CHK_PRM(jValue, "value field not found")

        std::string tmp;
        CHK_PRM(json_utils::val2str(jValue, tmp), "value field incorrect format")
        m_value = std::stoull(tmp);

        jValue = m_reader.get("fee", *params);
        if (jValue && json_utils::val2str(jValue, tmp)) {
            m_fee = std::stoull(tmp);
        }

        m_reader.get_value(*params, "data", m_data);

        CHK_PRM(storage::keys::peek(m_address, m_keys), "failed on get keys")

        return true;
    }
    END_TRY_RET(false)
}


bool create_tx_base_handler::build_request()
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

        m_writer.reset();
        m_writer.set_method("mhc_send");
        m_writer.add_param("transaction", transaction.c_str());
        m_writer.add_param("to", m_to.c_str());
        m_writer.add_param("value", std::to_string(m_value));
        m_writer.add_param("fee", !m_fee ? "" : std::to_string(m_fee));
        m_writer.add_param("nonce", std::to_string(m_nonce));
        m_writer.add_param("data", m_data.c_str());
        m_writer.add_param("pubkey", m_keys.pub_key);
        m_writer.add_param("sign", sign);

        return true;
    }
    END_TRY_RET(false)
}

