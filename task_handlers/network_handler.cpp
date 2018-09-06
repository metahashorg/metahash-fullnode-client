#include "task_handlers.h"
#include "../http_json_rpc_request.h"
#include "../wallet_storage/wallet_storage.h"

template <class T>
void base_network_handler<T>::execute()
{
    this->m_request->set_path(this->m_reader.get_method());
    this->m_request->set_body(this->m_writer.stringify());

    this->m_result.pending = this->m_async_execute;
    if (!this->m_async_execute)
    {
        this->m_request->execute();
        this->m_writer.reset();
        this->m_writer.parse(this->m_request->get_result());
    }
    else
    {
        auto request = this->m_request;
        auto session = this->m_session;
        json_rpc_id id = this->m_id;
        this->m_request->execute_async([id, request, session]()
        {
            json_rpc_reader reader;
            json_rpc_writer writer;
            writer.set_id(id);

            if (!reader.parse(request->get_result()))
                writer.set_error(-32603, "Invalid response json");
            else
            {
                json_rpc_id _id = reader.get_id();
                if (_id != 0 && _id != id)
                    writer.set_error(-32603, "Returned id doesn't match");
                else
                {
                    if (auto err = reader.get_error())
                        writer.set_error(*err);
                    else if (auto res = reader.get_result())
                        writer.set_result(*res);
                    else
                        writer.set_error(-32603, "No occur result or error");
                }
            }
            boost::asio::post(boost::bind(&http_session::send_json, session, writer.stringify()));
        });
    }
}

// base_send_tx_handler
template <class T>
bool base_send_tx_handler<T>::prepare_params()
{
    CHK_PRM(this->m_id, "id field not found")

    auto params = this->m_reader.get_params();
    CHK_PRM(params, "params field not found")

    CHK_PRM(this->m_reader.get_value(*params, "address", this->m_address) && !this->m_address.empty(), "address field not found")

    std::string to;
    CHK_PRM(this->m_reader.get_value(*params, "to", to) && !to.empty(), "to field not found")

    mh_count_t value(0);
    CHK_PRM(this->m_reader.get_value(*params, "value", value), "value field not found")

    mh_count_t fee(0);
    this->m_reader.get_value(*params, "fee", fee);

    mh_count_t nonce(0);
    CHK_PRM(this->get_nonce(nonce), "nonce not defined or its can't get")

    std::string data;
    this->m_reader.get_value(*params, "data", data);

    storage::crypt_keys keys;
    CHK_PRM(storage::keys::peek(this->m_address, keys), "failed on get keys")

    std::string sign;
    CHK_PRM(utils::gen_sign(sign, keys.prv_key, "xDDDd", to.c_str(), value, fee, nonce, data.size()), "failed on gen sign")

    this->m_writer.set_method("mhc_send");
    this->m_writer.add_param("to", to.c_str());
    this->m_writer.add_param("value", boost::lexical_cast<std::string>(value));
    this->m_writer.add_param("fee", !fee ? "" : boost::lexical_cast<std::string>(fee));
    this->m_writer.add_param("nonce", boost::lexical_cast<std::string>(nonce));
    this->m_writer.add_param("data", data.c_str());
    this->m_writer.add_param("pubkey", keys.pub_key);
    this->m_writer.add_param("sign", sign);

    return true;
}
