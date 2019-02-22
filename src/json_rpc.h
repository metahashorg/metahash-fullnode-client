#pragma once

#define RAPIDJSON_HAS_STDSTRING 1

#include <vector>
#include <string.h>
#include "rapidjson/document.h"

static const char json_rpc_ver[] = "2.0";

static const std::vector<std::string> json_rpc_service = {"id", "version", "result", "error"};

using json_rpc_id = uint32_t;

namespace json_utils
{
    bool val2str(rapidjson::Value* value, std::string& resut);
    void to_json(const std::string_view& param_list, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator);
}

class json_rpc_reader
{
public:
    json_rpc_reader();
    ~json_rpc_reader();

    bool parse(const std::string& json);
    std::string stringify(rapidjson::Value* value = nullptr);

    inline bool	is_valid() const { return !m_error.IsError(); }

    json_rpc_id get_id();
    std::string get_method();
    rapidjson::Value* get_error();
    rapidjson::Value* get_result();
    rapidjson::Value* get_params();
    rapidjson::Value* get(const std::string& name, rapidjson::Value& root);

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
    bool get_value(rapidjson::Value& root, const char* name, std::string_view& value) const;

    rapidjson::Document& get_doc();

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

    std::string stringify(rapidjson::Value* value = nullptr);

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
    
    template <typename T>
    void add(const std::string& name, T value)
    {
        rapidjson::Value& val = get_value(m_doc, name, rapidjson::kNullType);
        val.Set<T>(value, m_doc.GetAllocator());
    }

    void add_value(const std::string& name, rapidjson::Value& value)
    {
        get_value(m_doc, name, rapidjson::kNullType).CopyFrom(value, m_doc.GetAllocator());
    }
    
    rapidjson::Value* get_params();
    rapidjson::Value* new_value(const std::string& name);
    void push_back(rapidjson::Value& array, rapidjson::Value& value);
    
    rapidjson::Document::AllocatorType& get_allocator() { return m_doc.GetAllocator(); };

    void set_error(rapidjson::Value& value);
    void set_error(int code, const std::string& message);
    void set_id(json_rpc_id value);

    bool is_error() const;

    rapidjson::Value& get_value(rapidjson::Value& root, const std::string& name, rapidjson::Type Type);

    rapidjson::Document& getDoc() {
        return m_doc;
    }
    
protected:
    rapidjson::Document m_doc;
};
