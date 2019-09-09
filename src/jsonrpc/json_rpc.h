#ifndef __JSON_RPC_H__
#define __JSON_RPC_H__

#define RAPIDJSON_HAS_STDSTRING 1

#include <vector>
#include <string.h>
#include "rapidjson/document.h"
#include "rapidjson/schema.h"
#include "rapidjson/writer.h"

static const char json_rpc_ver[] = "2.0";

static const std::vector<std::string> json_rpc_service = {"id", "version", "result", "error"};

using json_rpc_id = uint32_t;

namespace json_utils
{
    bool val2str(const rapidjson::Value* value, std::string& resut);
    void to_json(const std::string_view& param_list, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator);
}

class json_rpc_reader
{
public:
    json_rpc_reader();
    ~json_rpc_reader();

    bool validate_request() const;

    bool parse(const char* json, size_t size);
    const std::string_view stringify(const rapidjson::Value* value = nullptr) const;

    inline rapidjson::ParseErrorCode get_parse_error() const { return m_error.Code(); };
    const char* get_parse_error_str() const;

    bool has_id() const;
    json_rpc_id get_id() const;
    const std::string_view get_method() const;
    const rapidjson::Value* get_error() const;
    const rapidjson::Value* get_result() const;
    const rapidjson::Value* get_params() const;
    const rapidjson::Value* get(const char* name, const rapidjson::Value& root) const;

    template <typename T>
    bool get_value(const rapidjson::Value& root, const char* name, T& value) const
    {
        if (!root.IsObject())
            return false;
        rapidjson::Value::ConstMemberIterator v = root.FindMember(name);
        if (v != root.MemberEnd() && v->value.Is<T>())
        {
            value = v->value.Get<T>();
            return true;
        }
        return false;
    }
    bool get_value(const rapidjson::Value& root, const char* name, std::string_view& value) const;
    const rapidjson::Document& get_doc() const;

protected:
    rapidjson::ParseResult  m_error;
    mutable rapidjson::Document m_doc;
    mutable rapidjson::StringBuffer m_buf;
};


class json_rpc_writer
{
public:
    json_rpc_writer();
    ~json_rpc_writer();

    bool parse(const char* json, size_t size);
    void reset();

    const std::string_view stringify(const rapidjson::Value* value = nullptr);

    void set_id(json_rpc_id value);
    void set_method(const char* value);
    void set_result(const rapidjson::Value& value);

    template <typename T>
    void add_result(const char* name, T value)
    {
        rapidjson::Value& result = get_value(m_doc, "result", rapidjson::kObjectType);
        rapidjson::Value& param = get_value(result, name, rapidjson::kNullType);
        param.Set<T>(value, m_doc.GetAllocator());
    }

    template <typename T>
    void add_param(const char* name, T value)
    {
        rapidjson::Value& params = get_value(m_doc, "params", rapidjson::kObjectType);
        rapidjson::Value& param = get_value(params, name, rapidjson::kNullType);
        param.Set<T>(value, m_doc.GetAllocator());
    }
    
    void add_value(const char* name, const rapidjson::Value& value)
    {
        get_value(m_doc, name, value.GetType()).CopyFrom(value, m_doc.GetAllocator());
    }
    
    bool is_error() const;
    void set_error(const rapidjson::Value& value);
    void set_error(int code, const char* message);

    template <typename T>
    void add_error_data(const char* name, T value)
    {
        rapidjson::Value& err = get_value(m_doc, "error", rapidjson::kObjectType);
        rapidjson::Value& data = get_value(err, "data", rapidjson::kObjectType);
        rapidjson::Value& param = get_value(data, name, rapidjson::kNullType);
        param.Set<T>(value, m_doc.GetAllocator());
    }

    rapidjson::Value* get_params();
    rapidjson::Value& get_value(rapidjson::Value& root, const char* name, rapidjson::Type Type);

    rapidjson::Document& get_doc();
    rapidjson::Document::AllocatorType& get_allocator();

protected:
    rapidjson::Document m_doc;
    rapidjson::StringBuffer m_buf;
};

#endif //__JSON_RPC_H__
