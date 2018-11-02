#pragma once

#include <string>
#include "network_handler.h"
#include "../wallet_storage/wallet_storage.h"

class create_tx_base_handler : public base_network_handler
{
public:
    create_tx_base_handler(http_session_ptr session):
        base_network_handler(settings::server::proxy, session),
        m_fee(0),
        m_value(0),
        m_nonce(0)
    {
    }

    virtual ~create_tx_base_handler() override { }

protected:
    bool check_params()
    {
        BGN_TRY
        {
            CHK_PRM(this->m_id, "id field not found")

            auto params = this->m_reader.get_params();
            CHK_PRM(params, "params field not found")

            CHK_PRM(this->m_reader.get_value(*params, "address", this->m_address) && !this->m_address.empty(), "address field not found")
            CHK_PRM(this->m_address.compare(0, 2, "0x") == 0, "address field must be in hex format")

            CHK_PRM(this->m_reader.get_value(*params, "to", this->m_to) && !this->m_to.empty(), "to field not found")
            CHK_PRM(this->m_to.compare(0, 2, "0x") == 0, "to field must be in hex format")

            auto jValue = this->m_reader.get("value", *params);
            CHK_PRM(jValue, "value field not found")

            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "value field incorrect format")

            this->m_value = std::stoull(tmp);

            jValue = this->m_reader.get("fee", *params);
            if (jValue && json_utils::val2str(jValue, tmp))
            {
                this->m_fee = std::stoull(tmp);
            }

            this->m_reader.get_value(*params, "data", this->m_data);
            
            CHK_PRM(storage::keys::peek(this->m_address, this->m_keys), "failed on get keys")

            return true;
        }
        END_TRY_RET(false)
    }

    bool build_request()
    {
        BGN_TRY
        {
            std::string sign;
            std::string transaction;
            CHK_PRM(utils::gen_sign(transaction, sign, this->m_keys.prv_key, "xDDDdx", this->m_to.c_str(), this->m_value, this->m_fee, this->m_nonce, this->m_data.size() / 2, this->m_data.c_str()), "failed on gen sign")

            this->m_writer.reset();
            this->m_writer.set_method("mhc_send");
            this->m_writer.add_param("transaction", transaction.c_str());
            this->m_writer.add_param("to", this->m_to.c_str());
            this->m_writer.add_param("value", boost::lexical_cast<std::string>(this->m_value));
            this->m_writer.add_param("fee", !this->m_fee ? "" : boost::lexical_cast<std::string>(this->m_fee));
            this->m_writer.add_param("nonce", boost::lexical_cast<std::string>(this->m_nonce));
            this->m_writer.add_param("data", this->m_data.c_str());
            this->m_writer.add_param("pubkey", this->m_keys.pub_key);
            this->m_writer.add_param("sign", sign);

            return true;
        }
        END_TRY_RET(false)
    }

protected:
    mh_count_t          m_fee;
    mh_count_t          m_value;
    mh_count_t          m_nonce;
    std::string         m_address;
    std::string         m_to;
    std::string         m_data;
    storage::crypt_keys m_keys;
};
