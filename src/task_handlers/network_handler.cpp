#include "network_handler.h"
#include "http_json_rpc_request.h"
#include "http_session.h"
#include <memory>
#include "common/string_utils.h"

base_network_handler::base_network_handler(const std::string &host, http_session_ptr session) 
    : base_handler(session)
{
    m_request = std::make_shared<http_json_rpc_request>(host, session->get_io_context());
}

base_network_handler::~base_network_handler()
{
}

void base_network_handler::execute()
{
    BGN_TRY
    {
        m_request->set_path(m_reader.get_method());
        m_request->set_body(m_writer.stringify());
        m_result.pending = m_async_execute;
        if (!m_async_execute) {
            m_request->execute();
            m_writer.reset();
            m_writer.parse(m_request->get_result());
        } else {
            auto self = shared_from(this);
            m_request->execute_async([self](){ self->on_complete(); });
        }
    }
    END_TRY
}

void base_network_handler::process_response(json_rpc_reader &reader)
{
    if (auto err = reader.get_error()) {
        m_writer.set_error(*err);
    } else if (auto res = reader.get_result()) {
        m_writer.set_result(*res);
    } else {
        CHK_PRM(false, "No occur result or error");
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

void base_network_handler::on_complete()
{
    BGN_TRY
    {
        m_writer.reset();
        json_rpc_reader reader;
        CHK_PRM(reader.parse(m_request->get_result()),
                string_utils::str_concat("Invalid response json: ", std::to_string(reader.get_parse_error().Code())))
        CHK_PRM(reader.get_error() || reader.get_result(), "No occur result or error")

        process_response(reader);
        send_response();
    }
    END_TRY_PARAM(send_response())
}

void base_network_handler::send_response()
{
    m_session->send_json(m_writer.stringify());
}
