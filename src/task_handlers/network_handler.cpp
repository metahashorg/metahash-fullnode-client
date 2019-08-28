#include "network_handler.h"
#include "http_json_rpc_request.h"
#include "http_session.h"
#include <memory>
#include "common/string_utils.h"

base_network_handler::base_network_handler(const std::string &host, session_context_ptr session_ctx)
    : base_handler(session_ctx)
{
    m_name = __func__;
    boost::asio::io_context* ctx = nullptr;
    if (session_ctx) {
        ctx = &session_ctx->get_io_context();
    } else {
        m_ioctx.reset(new boost::asio::io_context());
        ctx = m_ioctx.get();
    }
    m_request = std::make_shared<http_json_rpc_request>(host, *ctx);
}

base_network_handler::~base_network_handler()
{
}

void base_network_handler::execute()
{
    BGN_TRY
    {
        m_request->set_path(m_reader.get_method().data());
        m_request->set_body(m_writer.stringify().data());
        if (!m_context) {
            m_async_execute = false;
        }
        m_result.pending = m_async_execute;
        LOGINFO << "[" << get_name() << "] Async execute " << m_async_execute;
        if (!m_async_execute) {
            m_request->execute();
            m_writer.reset();
            m_writer.parse(m_request->get_result().data());
        } else {
            auto self = shared_from(this);
            m_request->execute_async([self](){ self->on_complete(); });
        }
    }
    END_TRY()
}

void base_network_handler::process_response(json_rpc_reader &reader)
{
    if (const rapidjson::Value* err = reader.get_error()) {
        m_writer.set_error(*err);
    } else if (const rapidjson::Value* res = reader.get_result()) {
        m_writer.set_result(*res);
    } else {
        CHK_PRM(false, "No occur result or error");
    }

    bool skip = true;
    const rapidjson::Document& doc = reader.get_doc();
    for (const auto& m : doc.GetObject()) {
//        std::string name = m.name.GetString();
//        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
//        if (std::find(json_rpc_service.begin(), json_rpc_service.end(), name) != json_rpc_service.end()) {
//            continue;
//        }
        skip = false;
        for (const auto& v: json_rpc_service){
            if (v.size() != m.name.GetStringLength()) {
                continue;
            }
            if (v.compare(m.name.GetString()) == 0) {
                skip = true;
                break;
            }
        }
        if (skip) {
            continue;
        }
        m_writer.add_value(m.name.GetString(), m.value);
    }
}

void base_network_handler::on_complete()
{
    BGN_TRY
    {
        LOGINFO << "[" << get_name() << "] Complete async execution";
        m_writer.reset();
        json_rpc_reader reader;
        CHK_PRM(reader.parse(m_request->get_result().data()),
                string_utils::str_concat("Invalid response json, parse error (", std::to_string(reader.get_parse_error()),
                                         "): ", reader.get_parse_error_str()))
        CHK_PRM(reader.get_error() || reader.get_result(), "No occur result or error")

        process_response(reader);
        send_response();
    }
    END_TRY(send_response())
}

void base_network_handler::send_response()
{
    BGN_TRY
    {
        std::string_view result = m_writer.stringify();
        m_context->send_json(result.data(), result.size());
    }
    END_TRY()
}
