#include "send_tx_handler.h"

#include "../SyncSingleton.h"
#include "../sync/BlockInfo.h"
#include "settings/settings.h"
#include "http_session.h"
#include "http_json_rpc_request.h"
#include "common/string_utils.h"
#include "cpplib_open_ssl_decor/crypto.h"
#include "utils.h"
#include "task_handlers/utils.h"

bool send_tx_handler::prepare_params()
{
    BGN_TRY
    {
        switch (send_tx_handler::check_params()) {
            case -1:
                // error
                return false;

            case 1:
            {
                mh_count_t data_size = 0;
                CHK_PRM(utils::parse_tansaction(m_transaction, m_to, m_value, m_fee, m_nonce, data_size, m_data), "failed on parse transaction")

                std::vector<unsigned char> data_bin = hex2bin(m_transaction);
                std::vector<unsigned char> sign_bin;
                CRYPTO_sign_data(sign_bin, m_keys.prv_key, data_bin);
                m_sign = bin2hex(sign_bin);

                CHK_PRM(utils::make_tx(m_hash, "xuxux", m_transaction.c_str(), m_sign.size() / 2, m_sign.c_str(), m_keys.pub_key.size() / 2, m_keys.pub_key.c_str()), "failed on generate tx")

                make_json();
            }
            break;

            default:
            {
                auto params = m_reader.get_params();
                auto jValue = m_reader.get("nonce", *params);
                if (jValue) {
                    std::string tmp;
                    CHK_PRM(json_utils::val2str(jValue, tmp), "'nonce' field has incorrect format")
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

                    auto request = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), m_context->get_io_context());
                    request->set_path("fetch-balance");
                    request->set_body(writer.stringify().data());

                    auto self = shared_from(this);
                    request->execute_async([self, request](){ self->on_get_balance(request); });

                    return false;
                }

                if (!build_request(true)) {
                    return false;
                }
            }
            break;
        }

        return true;
    }
    END_TRY(return false)
}

void send_tx_handler::on_get_balance(http_json_rpc_request_ptr request)
{
    BGN_TRY
    {
        json_rpc_reader reader;

        const std::string_view str = request->get_result();
        CHK_PRM(reader.parse(str.data(), str.size()),
                string_utils::str_concat("fetch-balane response parse error (", std::to_string(reader.get_parse_error()),
                                         "): ", reader.get_parse_error_str()).c_str())

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
        CHK_PRM(reader.get_value(*res, "count_spent", count_spent), "fetch-balane response: field 'count_spent' not found")
        m_nonce = count_spent + 1;

        if (!build_request(true)) {
            send_response();
        } else {
            send_tx_handler::execute();
        }
    }
    END_TRY(send_response())
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

int send_tx_handler::check_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        m_reader.get_value(*params, "transaction", m_transaction);
        if (m_transaction.empty()) {
            return create_tx_base_handler::check_params() ? 0 : -1;
        }

        CHK_PRM(m_reader.get_value(*params, "address", m_address), "address field not found");
        CHK_PRM(!m_address.empty(), "address field not found")
        CHK_PRM(utils::validate_address(m_address), "address is invalid")
        CHK_PRM(storage::keys::peek(m_address, m_keys), "failed on get keys")

        return 1;
    }
    END_TRY(return -1)
}

int send_tx_handler::check_params_1()
{
    BGN_TRY
    {
        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "to", m_to), "to field not found");
        CHK_PRM(utils::validate_address(m_to), "reciever address is invalid")
        //CHK_PRM(m_reader.get_value(*params, "sign", m_sign) && !m_sign.empty(), "sign field not found")

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
        return 1;
    }
    END_TRY(return -1)
}

