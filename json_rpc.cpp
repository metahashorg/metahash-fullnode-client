#include "json_rpc.h"
#include "log/log.h"

// json_rpc_reader

json_rpc_reader::json_rpc_reader()
{
}

json_rpc_reader::~json_rpc_reader()
{
}

bool json_rpc_reader::parse(const std::string& json)
{
    try
    {
        m_error = m_doc.Parse(json.c_str());
        return !m_error.IsError();
    }
    catch (const std::exception& e)
    {
        logg::push_err(e.what());
        m_error.Set(rapidjson::ParseErrorCode::kParseErrorTermination);
        return false;
    }
}

json_rpc_id json_rpc_reader::get_id()
{
    json_rpc_id res(0);
    get_value(m_doc, "id", res);
    return res;
}

std::string json_rpc_reader::get_method()
{
    std::string res;
    get_value(m_doc, "method", res);
    return res;
}

rapidjson::Value* json_rpc_reader::get_error()
{
    auto p = m_doc.FindMember("error");
    if (p == m_doc.MemberEnd())
        return nullptr;
    return &p->value;
}

rapidjson::Value* json_rpc_reader::get_result()
{
    auto p = m_doc.FindMember("result");
    if (p == m_doc.MemberEnd())
        return nullptr;
    return &p->value;
}

rapidjson::Value* json_rpc_reader::get_params()
{
    auto p = m_doc.FindMember("params");
    if (p == m_doc.MemberEnd())
        return nullptr;
    return &p->value;
}

// json_rpc_writer

json_rpc_writer::json_rpc_writer()
{
    reset();
}

json_rpc_writer::~json_rpc_writer()
{
}

bool json_rpc_writer::parse(const std::string& json)
{
    try
    {
        m_doc.Parse(json.c_str());
        return !m_doc.HasParseError();
    }
    catch (const std::exception& e)
    {
        logg::push_err(e.what());
        return false;
    }
}

void json_rpc_writer::set_method(const std::string& value)
{
    get_value(m_doc, "method", rapidjson::kStringType).SetString(value, m_doc.GetAllocator());
}

void json_rpc_writer::set_result(rapidjson::Value& value)
{
    get_value(m_doc, "result", value.GetType()) = value;
}

void json_rpc_writer::set_error(int code, std::string message)
{
    rapidjson::Value err(rapidjson::kObjectType);

    err.AddMember("code", rapidjson::kNumberType, m_doc.GetAllocator());
    err["code"].SetInt(code);
    err.AddMember("message", rapidjson::kStringType, m_doc.GetAllocator());
    err["message"].SetString(message, m_doc.GetAllocator());

    get_value(m_doc, "error", rapidjson::kObjectType) = err;
}

void json_rpc_writer::set_error(rapidjson::Value& value)
{
    get_value(m_doc, "error", value.GetType()) = value;
}

void json_rpc_writer::set_id(json_rpc_id value)
{
    if (value == 0)
        get_value(m_doc, "id", rapidjson::kNullType).SetNull();
    else
        get_value(m_doc, "id", rapidjson::kNumberType).Set<json_rpc_id>(value);
}

std::string json_rpc_writer::stringify()
{

    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    m_doc.Accept(writer);
    return buf.GetString();
}

rapidjson::Value& json_rpc_writer::get_value(rapidjson::Value& root, const std::string& name, rapidjson::Type Type)
{
    auto node = root.FindMember(name);
    if (node != root.MemberEnd())
        return node->value;
    rapidjson::Value n(name, m_doc.GetAllocator());
    root.AddMember(n, Type, m_doc.GetAllocator());
    return root[name];
}

void json_rpc_writer::reset()
{
    m_doc.SetObject();
    get_value(m_doc, "jsonrpc", rapidjson::kStringType) = json_rpc_ver;
}

bool json_rpc_writer::is_error() const
{
    return m_doc.HasMember("error");
}
