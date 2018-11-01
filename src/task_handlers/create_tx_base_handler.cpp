#include "task_handlers.h"
#include "create_tx_base_handler.h"
#include "../http_json_rpc_request.h"

// create_tx_base_handler
//template <class T>
//create_tx_base_handler<T>::create_tx_base_handler(http_session_ptr session):
//    base_network_handler<T>(settings::server::proxy, session),
//    m_fee(0),
//    m_value(0),
//    m_nonce(0)
//{
//}

//template <class T>
//bool create_tx_base_handler<T>::check_params()
//{
//    try
//    {
//        CHK_PRM(this->m_id, "id field not found")

//        auto params = this->m_reader.get_params();
//        CHK_PRM(params, "params field not found")

//        CHK_PRM(this->m_reader.get_value(*params, "address", this->m_address) && !this->m_address.empty(), "address field not found")
//        CHK_PRM(this->m_address.compare(0, 2, "0x") == 0, "address field must be in hex format")

//        CHK_PRM(this->m_reader.get_value(*params, "to", this->m_to) && !this->m_to.empty(), "to field not found")
//        CHK_PRM(this->m_to.compare(0, 2, "0x") == 0, "to field must be in hex format")

//        auto jValue = this->m_reader.get("value", *params);
//        CHK_PRM(jValue, "value field not found")

//        std::string tmp;
//        CHK_PRM(json_utils::val2str(jValue, tmp), "value field incorrect format")

//        this->m_value = std::stoull(tmp);

//        jValue = this->m_reader.get("fee", *params);
//        if (jValue && json_utils::val2str(jValue, tmp))
//        {
//            this->m_fee = std::stoull(tmp);
//        }

//        this->m_reader.get_value(*params, "data", this->m_data);

//        CHK_PRM(storage::keys::peek(this->m_address, this->m_keys), "failed on get keys")

//        return true;
//    }
//    catch (std::exception& ex)
//    {
//        STREAM_LOG_ERR("check params exception: " << ex.what())
//        std::string out("check params exception: ");
//        out.append(ex.what());
//        this->m_writer.set_error(-32603, out);
//        return false;
//    }
//    catch(...)
//    {
//        STREAM_LOG_ERR("check params unknown exception")
//        this->m_writer.set_error(-32603, "check params unknown exception");
//        return false;
//    }
//}

//template <class T>
//bool create_tx_base_handler<T>::build_request()
//{
//    try
//    {
//        std::string sign;
//        CHK_PRM(utils::gen_sign(sign, this->m_keys.prv_key, "xDDDd", this->m_to.c_str(), this->m_value, this->m_fee, this->m_nonce, this->m_data.size()), "failed on gen sign")

//        this->m_writer.set_method("mhc_send");
//        this->m_writer.add_param("to", this->m_to.c_str());
//        this->m_writer.add_param("value", boost::lexical_cast<std::string>(this->m_value));
//        this->m_writer.add_param("fee", !this->m_fee ? "" : boost::lexical_cast<std::string>(this->m_fee));
//        this->m_writer.add_param("nonce", boost::lexical_cast<std::string>(this->m_nonce));
//        this->m_writer.add_param("data", this->m_data.c_str());
//        this->m_writer.add_param("pubkey", this->m_keys.pub_key);
//        this->m_writer.add_param("sign", sign);

//        return true;
//    }
//    catch (std::exception& ex)
//    {
//        STREAM_LOG_ERR("build request exception: " << ex.what())
//        std::string out("build request exception: ");
//        out.append(ex.what());
//        this->m_writer.set_error(-32603, out);
//        return false;
//    }
//    catch(...)
//    {
//        STREAM_LOG_ERR("build request unknown exception")
//        this->m_writer.set_error(-32603, "build request unknown exception");
//        return false;
//    }
//}
