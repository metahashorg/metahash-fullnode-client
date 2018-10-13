#include "json_rpc.h"
#include "log/log.h"
#include <sstream>

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

std::string json_rpc_reader::stringify(rapidjson::Value* value /*= nullptr*/)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    if (!value)
        value = &m_doc;
    value->Accept(writer);
    return buf.GetString();
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
return get("error", m_doc);
}

rapidjson::Value* json_rpc_reader::get_result()
{
    return get("result", m_doc);
}

rapidjson::Value* json_rpc_reader::get_params()
{
    return get("params", m_doc);
}

rapidjson::Value* json_rpc_reader::get(const std::string& name, rapidjson::Value& root)
{
    auto p = root.FindMember(name);
    if (p == root.MemberEnd())
        return nullptr;
    return &p->value;
}

rapidjson::Document& json_rpc_reader::get_doc()
{
    return m_doc;
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

std::string json_rpc_writer::stringify(rapidjson::Value* value /*= nullptr*/)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    if (!value)
        value = &m_doc;
    value->Accept(writer);
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
    get_value(m_doc, "id", rapidjson::kNullType).SetNull();
}

bool json_rpc_writer::is_error() const
{
    return m_doc.HasMember("error");
}

rapidjson::Value* json_rpc_writer::get_params()
{
    auto p = m_doc.FindMember("params");
    if (p == m_doc.MemberEnd())
    {
	return &get_value(m_doc, "params", rapidjson::kObjectType);
    }
    return &p->value;
}

rapidjson::Value* json_rpc_writer::new_value(const std::string& name)
{
    return new rapidjson::Value(name, m_doc.GetAllocator());
}

void json_rpc_writer::push_back(rapidjson::Value& array, rapidjson::Value& value)
{
    array.PushBack(value, m_doc.GetAllocator());
}

namespace json_utils
{
    bool val2str(rapidjson::Value* value, std::string& result)
    {
        result.clear();

        if (value == nullptr)
            return false;

        if (value->IsString())
        {
            result = value->GetString();
            return true;
        }

        std::ostringstream out;
        if (value->IsInt())
        {
            out << value->GetInt();
        }
        else if (value->IsInt64())
        {
            out << value->GetInt64();
        }
        else if (value->IsUint())
        {
            out << value->GetUint();
        }
        else if (value->IsUint64())
        {
            out << value->GetUint64();
        }
        else if (value->IsDouble())
        {
            out << value->GetDouble();
        }
        else if (value->IsFloat())
        {
            out << value->GetFloat();
        }
        else
        {
            return false;
        }
        out.flush();
        result = out.str();
        return true;
    }
}
