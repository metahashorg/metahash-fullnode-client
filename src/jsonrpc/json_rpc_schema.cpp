#include "json_rpc_schema.h"
#include "rapidjson/error/en.h"
#include "log.h"

extern char _schema_request_json_start[] asm( "_binary_jsonrpc_schema_schema_request_json_start" );
extern char _schema_request_json_end[]   asm( "_binary_jsonrpc_schema_schema_request_json_end" );


std::map<jsonrpc_schema::type, std::unique_ptr<rapidjson::SchemaDocument> > jsonrpc_schema::m_schemas;
std::mutex jsonrpc_schema::m_locker;

jsonrpc_schema::jsonrpc_schema()
{
}

jsonrpc_schema::~jsonrpc_schema()
{
}

void jsonrpc_schema::free()
{
    m_schemas.clear();
}

const rapidjson::SchemaDocument* jsonrpc_schema::get(type schema_type)
{
    auto it = m_schemas.find(schema_type);
    if (it == m_schemas.end()) {
        std::lock_guard<std::mutex> lock(m_locker);
        it = m_schemas.find(schema_type);
        if (it == m_schemas.end()) {
            rapidjson::Document doc;
            switch (schema_type) {
                case request:
                {
                    doc.Parse(_schema_request_json_start, _schema_request_json_end - _schema_request_json_start);
                    if (doc.HasParseError()) {
                        LOGERR << "Schema " << request << " parse error: " << doc.GetErrorOffset() << " " << rapidjson::GetParseError_En(doc.GetParseError());
                    } else {
                        auto result = m_schemas.emplace(request, new rapidjson::SchemaDocument(doc));
                        if (!result.second) {
                            LOGERR << "Schema " << request << " error: couldn't insert into map";
                        } else {
                            LOGINFO << "Json schema " << request << " registered.";
                            return result.first->second.get();
                        }
                    }
                }
                break;

            default:
                break;
            }
            return nullptr;
        }
    }
    return it->second.get();
}
