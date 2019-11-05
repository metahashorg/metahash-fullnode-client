#ifndef __JSON_RPC_SCHEMA_H__
#define __JSON_RPC_SCHEMA_H__

#include <map>
#include <memory>
#include <mutex>
#include "rapidjson/document.h"
#include "rapidjson/schema.h"
#include "singleton.h"

class jsonrpc_schema: public singleton<jsonrpc_schema>
{
    friend class singleton<jsonrpc_schema>;

    jsonrpc_schema();
public:
    enum type {
        request,
        methods
    };

    const rapidjson::SchemaDocument* get_schema(type schema_type);

private:
    const rapidjson::SchemaDocument* load(type schema_type);

private:
    std::map<type, std::unique_ptr<rapidjson::SchemaDocument> > m_schemas;
};


#endif // __JSON_RPC_SCHEMA_H__

