#include "send_tx_handler.h"

// send_tx_handler
bool send_tx_handler::prepare_params()
{
    BGN_TRY
    {
        if (!this->check_params())
            return false;

        auto params = this->m_reader.get_params();
        auto jValue = this->m_reader.get("nonce", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "nonce field incorrect format")
            this->m_nonce = std::stoull(tmp);
        }
        else
        {
            this->m_result.pending = true;

            json_rpc_id id = 1;
            json_rpc_writer writer;
            writer.set_id(id);
            writer.add_param("address", this->m_address);

            auto request = std::make_shared<http_json_rpc_request>(settings::server::tor, this->m_session->get_io_context());
            request->set_path("fetch-balance");
            request->set_body(writer.stringify());
            request->execute_async(boost::bind(&send_tx_handler::on_get_balance, shared_from_this(), request, id));

            return false;
        }

        if (!this->build_request())
            return false;

        return true;
    }
    END_TRY_RET(false)
}

void send_tx_handler::on_get_balance(http_json_rpc_request_ptr request, json_rpc_id id)
{
    json_rpc_reader reader;
    json_rpc_writer writer;
    writer.set_id(id);

    if (!reader.parse(request->get_result()))
    {
        writer.set_error(-32605, "Invalid response json");
    }
    else
    {
        json_rpc_id _id = reader.get_id();
        if (_id != 0 && _id != id)
        {
            writer.set_error(-32605, "Returned id doesn't match");
        }
        else
        {
            if (auto err = reader.get_error())
            {
                writer.set_error(*err);
            }
            else if (auto res = reader.get_result())
            {
                mh_count_t count_spent(0);
                if (reader.get_value(*res, "count_spent", count_spent))
                {
                    this->m_nonce = count_spent + 1;
                }
                else
                {
                    writer.set_error(-32605, "field spent count not found");
                }
            }
            else
            {
                writer.set_error(-32605, "No occur result or error");
            }
        }
    }

    this->build_request();

    boost::asio::post(boost::bind(&send_tx_handler::execute, shared_from_this()));
}
