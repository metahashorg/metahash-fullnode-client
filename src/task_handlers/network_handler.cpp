#include "network_handler.h"

#include <memory>

#include "http_json_rpc_request.h"
#include "log/log.h"
#include "http_session.h"

base_network_handler::base_network_handler(const std::string &host, http_session_ptr session) 
    : base_handler(session)
{
    m_request = std::make_shared<http_json_rpc_request>(host, session->get_io_context());
}

void base_network_handler::execute()
{
    BGN_TRY
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
            this->m_request->execute_async(boost::bind(&base_network_handler::on_complete, this->shared_from_this(), this->m_id, this->m_request, this->m_session));
        }
    }
    END_TRY
}

void base_network_handler::processResponse(json_rpc_id id, http_json_rpc_request_ptr req) {
    json_rpc_reader reader;
   
    CHK_PRM(reader.parse(req->get_result()), "Invalid response json")
    
    json_rpc_id _id = reader.get_id();
    
    //CHK_PRM(_id != 0 && _id == id, "Returned id doesn't match")
    
    auto err = reader.get_error();
    auto res = reader.get_result();
    
    CHK_PRM(err || res, "No occur result or error")
    
    if (err) {
        this->m_writer.set_error(*err);
    } else if (res) {
        this->m_writer.set_result(*res);
    }
    
    rapidjson::Document& doc = reader.get_doc();
    for (auto& m : doc.GetObject()) {
        std::string name = m.name.GetString();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (std::find(json_rpc_service.begin(), json_rpc_service.end(), name) != json_rpc_service.end()) {
            continue;
        }
        this->m_writer.add_value(m.name.GetString(), m.value);
    }
}

void base_network_handler::on_complete(json_rpc_id id, http_json_rpc_request_ptr req, http_session_ptr session)
{
    BGN_TRY
    {
        this->m_writer.reset();
        
        processResponse(id, req);

        boost::asio::post(boost::bind(&http_session::send_json, session, this->m_writer.stringify()));
    }
    END_TRY_PARAM(boost::asio::post(boost::bind(&http_session::send_json, session, this->m_writer.stringify())))
}
