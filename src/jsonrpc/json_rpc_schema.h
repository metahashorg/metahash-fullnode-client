#ifndef __JSON_RPC_SCHEMA_H__
#define __JSON_RPC_SCHEMA_H__

#include <map>
#include <memory>
#include <mutex>
#include "rapidjson/document.h"
#include "rapidjson/schema.h"

class jsonrpc_schema {
    jsonrpc_schema();
    ~jsonrpc_schema();
public:
    enum type {
        request,
        methods
    };

    static void free();
    static const rapidjson::SchemaDocument* get(type schema_type);

private:
    static const rapidjson::SchemaDocument* load(type schema_type);

private:
    static std::mutex m_locker;
    static std::map<type, std::unique_ptr<rapidjson::SchemaDocument> > m_schemas;
};


#endif // __JSON_RPC_SCHEMA_H__

