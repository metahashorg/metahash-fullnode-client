#include "send_tx_handler.h"

#include "../SyncSingleton.h"
#include "../sync/BlockInfo.h"
#include "settings/settings.h"
#include "http_session.h"
#include "http_json_rpc_request.h"
#include "common/string_utils.h"
#include "task_handlers/utils.h"

bool send_tx_handler::prepare_params()
{
    BGN_TRY
    {
        get_type();
        switch (m_type) {
            case 1:
            {
                if (!check_send_params()) {
                    return false;
                }

                /*
                uint64_t data_size = 0;
                CHK_PRM(utils::parse_tansaction(m_transaction, m_to, m_value, m_fee, m_nonce, data_size, m_data), "invalid transaction");
                if (!build_request()) {
                    return false;
                }
                */

                m_writer.reset();
                m_writer.set_method("mhc_send");
                m_writer.add_param("transaction", m_transaction.c_str());
                m_writer.add_param("to", m_to.c_str());
                m_writer.add_param("value", std::to_string(m_value));
                m_writer.add_param("fee", !m_fee ? "" : std::to_string(m_fee));
                m_writer.add_param("nonce", std::to_string(m_nonce));
                m_writer.add_param("data", m_data.c_str());
                m_writer.add_param("pubkey", m_pubkey);
                m_writer.add_param("sign", m_sign);

            }
            break;

            default:
            {
                if (!check_params()) {
                    return false;
                }

                auto params = m_reader.get_params();
                auto jValue = m_reader.get("nonce", *params);
                if (jValue) {
                    std::string tmp;
                    CHK_PRM(json_utils::val2str(jValue, tmp), "nonce field has incorrect format")
                    m_nonce = std::stoull(tmp);
                } else if (settings::system::useLocalDatabase) {
                    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
                    const torrent_node_lib::Sync &sync = *syncSingleton();
                    const torrent_node_lib::BalanceInfo balance = sync.getBalance(torrent_node_lib::Address(m_address));
                    m_nonce = balance.countSpent + 1;
                } else {
                    m_result.pending = true;

                    json_rpc_writer writer;
                    writer.set_id(1);
                    writer.add_param("address", m_address);

                    auto request = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), m_session->get_io_context());
                    request->set_path("fetch-balance");
                    request->set_body(writer.stringify());

                    auto self = shared_from(this);
                    request->execute_async([self, request](){ self->on_get_balance(request); });

                    return false;
                }

                if (!build_request()) {
                    return false;
                }
            }
            break;
        }

        return true;
    }
    END_TRY_RET(false)
}

void send_tx_handler::on_get_balance(http_json_rpc_request_ptr request)
{
    BGN_TRY
    {
        json_rpc_reader reader;

        CHK_PRM(reader.parse(request->get_result()),
                string_utils::str_concat("fetch-balane response parse error: ", std::to_string(reader.get_parse_error().Code())).c_str())

        auto err = reader.get_error();
        auto res = reader.get_result();

        CHK_PRM(err || res, "No occur result or error")

        if (err) {
            m_writer.reset();
            m_writer.set_error(*err);
            send_response();
            return;
        }

        mh_count_t count_spent(0);
        CHK_PRM(reader.get_value(*res, "count_spent", count_spent), "fetch-balane response: field spent count not found")
        m_nonce = count_spent + 1;

        if (!build_request()) {
            send_response();
        } else {
            send_tx_handler::execute();
        }
    }
    END_TRY_PARAM(send_response())
}

void send_tx_handler::process_response(json_rpc_reader &reader)
{
    auto err = reader.get_error();
    auto params = reader.get_params();

    if (err) {
        m_writer.set_error(*err);
    } else {
        CHK_PRM(params, "send-tx response: params field not found")
        CHK_PRM(params->IsString(), "send-tx response: params field has incorrect format");
        m_writer.set_result(*params);
    }
}

bool send_tx_handler::check_send_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "transaction", m_transaction) && !m_transaction.empty(), "transaction field not found")

        CHK_PRM(m_reader.get_value(*params, "to", m_to) && !m_to.empty(), "to field not found")
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

        jValue = m_reader.get("nonce", *params);
        CHK_PRM(jValue, "nonce field not found")
        tmp.clear();
        CHK_PRM(json_utils::val2str(jValue, tmp), "nonce field has incorrect format")
        m_nonce = std::stoull(tmp);

        m_reader.get_value(*params, "data", m_data);

        CHK_PRM(m_reader.get_value(*params, "pubkey", m_pubkey) && !m_pubkey.empty(), "pubkey field not found")
        CHK_PRM(m_reader.get_value(*params, "sign", m_sign) && !m_sign.empty(), "sign field not found")

        return true;
    }
    END_TRY_RET(false)
}

void send_tx_handler::get_type()
{
    BGN_TRY
    {
        auto params = m_reader.get_params();
        if (!params) {
            return;
        }
        m_reader.get_value(*params, "type", m_type);
    }
    END_TRY
}

