#include "json_rpc_schema.h"
#include "rapidjson/error/en.h"
#include "log.h"

extern char _schema_request_json_start[] asm( "_binary_jsonrpc_schema_schema_request_json_start" );
extern char _schema_request_json_end[]   asm( "_binary_jsonrpc_schema_schema_request_json_end" );
extern char _schema_methods_json_start[] asm( "_binary_jsonrpc_schema_schema_methods_json_start" );
extern char _schema_methods_json_end[]   asm( "_binary_jsonrpc_schema_schema_methods_json_end" );

jsonrpc_schema::jsonrpc_schema()
{
    load(request);
    load(methods);
}

const rapidjson::SchemaDocument* jsonrpc_schema::get_schema(type schema_type)
{
    auto it = m_schemas.find(schema_type);
    if (it == m_schemas.end()) {
        return nullptr;
    }
    return it->second.get();
}

const rapidjson::SchemaDocument* jsonrpc_schema::load(type schema_type)
{
    rapidjson::Document doc;
    switch (schema_type) {
        case request:
            doc.Parse(_schema_request_json_start, _schema_request_json_end - _schema_request_json_start);
            break;
        case methods:
            doc.Parse(_schema_methods_json_start, _schema_methods_json_end - _schema_methods_json_start);
            break;
        default:
            return nullptr;
    }
    if (doc.HasParseError()) {
        LOGERR << "Could not parse json schema #" << schema_type << " : " << doc.GetErrorOffset() << " " << rapidjson::GetParseError_En(doc.GetParseError());
    } else {
        auto result = m_schemas.emplace(schema_type, new rapidjson::SchemaDocument(doc));
        if (!result.second) {
            LOGERR << "Could not add json schema #" << schema_type << " into map";
        } else {
            LOGINFO << "Json schema #" << schema_type << " loaded.";
            return result.first->second.get();
        }
    }
    return nullptr;
}
