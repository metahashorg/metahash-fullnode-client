#include "network_handler.h"
#include "http_json_rpc_request.h"
#include "http_session.h"
#include <memory>

base_network_handler::base_network_handler(const std::string &host, http_session_ptr session) 
    : base_handler(session)
{
    m_request = std::make_shared<http_json_rpc_request>(host, session->get_io_context());
    int a = m_request->weak_from_this().use_count();
    a = 0;
}

base_network_handler::~base_network_handler()
{
    auto a = m_request->weak_from_this();
    int b = a.use_count();
    m_request.reset();
    b = a.use_count();
    b = 0;
}

void base_network_handler::execute()
{
    BGN_TRY
    {
        int a = m_request->weak_from_this().use_count();
        m_request->set_path(m_reader.get_method());
        a = m_request->weak_from_this().use_count();
        m_request->set_body(m_writer.stringify());
        a = m_request->weak_from_this().use_count();
        m_result.pending = m_async_execute;
        if (!m_async_execute) {
            m_request->execute();
            m_writer.reset();
            m_writer.parse(m_request->get_result());
        } else {
            a = m_request->weak_from_this().use_count();
            auto b = shared_from(this);
            a = m_request->weak_from_this().use_count();
            m_request->execute_async(boost::bind(&base_network_handler::on_complete, b, m_id));
            a = m_request->weak_from_this().use_count();
        }
        a = 0;
    }
    END_TRY
}

void base_network_handler::processResponse(json_rpc_id id, json_rpc_reader &reader) {
    //json_rpc_id _id = reader.get_id();
    //CHK_PRM(_id != 0 && _id == id, "Returned id doesn't match")
    
    auto err = reader.get_error();
    auto res = reader.get_result();
    
    CHK_PRM(err || res, "No occur result or error")
    
    if (err) {
        m_writer.set_error(*err);
    } else if (res) {
        m_writer.set_result(*res);
    }
    
    rapidjson::Document& doc = reader.get_doc();
    for (auto& m : doc.GetObject()) {
        std::string name = m.name.GetString();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (std::find(json_rpc_service.begin(), json_rpc_service.end(), name) != json_rpc_service.end()) {
            continue;
        }
        m_writer.add_value(m.name.GetString(), m.value);
    }
}

void base_network_handler::on_complete(json_rpc_id id)
{
    BGN_TRY
    {
        m_writer.reset();
        json_rpc_reader reader;
        CHK_PRM(reader.parse(m_request->get_result()), "Invalid response json")

        processResponse(id, reader);
        m_session->send_json(m_writer.stringify());
//        boost::asio::post(boost::bind(&http_session::send_json, session, m_writer.stringify()));
    }
    END_TRY_PARAM(m_session->send_json(m_writer.stringify()))
//    END_TRY_PARAM(boost::asio::post(boost::bind(&http_session::send_json, session, m_writer.stringify())))
}
