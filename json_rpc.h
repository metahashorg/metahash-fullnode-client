#pragma once

#define RAPIDJSON_HAS_STDSTRING 1

#include <string.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

static const char json_rpc_ver[] = "2.0";

using json_rpc_id = uint32_t;

class json_rpc_reader
{
public:
    json_rpc_reader();
    ~json_rpc_reader();

    bool parse(const std::string& json);

    inline bool	is_valid() const { return !m_error.IsError(); }

    json_rpc_id get_id();
    std::string get_method();
    rapidjson::Value* get_error();
    rapidjson::Value* get_result();
    rapidjson::Value* get_params();

    inline rapidjson::ParseResult get_parse_error() { return m_error; };

    template <typename T>
    bool get_value(rapidjson::Value& root, const std::string& name, T& value) const
    {
        if (!root.IsObject())
            return false;
        auto v = root.FindMember(name);
        if (v != root.MemberEnd() && v->value.Is<T>())
        {
            value = v->value.Get<T>();
            return true;
        }
        return false;
    }

protected:
    rapidjson::ParseResult  m_error;
    rapidjson::Document     m_doc;
};


class json_rpc_writer
{
public:
    json_rpc_writer();
    ~json_rpc_writer();

    bool parse(const std::string& json);
    void reset();

    std::string stringify();

    void set_method(const std::string& value);
    void set_result(rapidjson::Value& value);

    template <typename T>
    void add_result(const std::string& name, T value)
    {
        rapidjson::Value& result = get_value(m_doc, "result", rapidjson::kObjectType);
        rapidjson::Value& param = get_value(result, name, rapidjson::kNullType);
        param.Set<T>(value, m_doc.GetAllocator());
    }

    template <typename T>
    void add_param(const std::string& name, T value)
    {
        rapidjson::Value& params = get_value(m_doc, "params", rapidjson::kObjectType);
        rapidjson::Value& param = get_value(params, name, rapidjson::kNullType);
        param.Set<T>(value, m_doc.GetAllocator());
    }

    void set_error(rapidjson::Value& value);
    void set_error(int code, std::string message);
    void set_id(json_rpc_id value);

    bool is_error() const;

protected:
    rapidjson::Value& get_value(rapidjson::Value& root, const std::string& name, rapidjson::Type Type);

protected:
    rapidjson::Document m_doc;
};
