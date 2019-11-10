#include "base_handler.h"
#include "string_utils.h"
#include "settings/settings.h"
#include "jsonrpc/json_rpc_schema.h"

bool base_handler::prepare(const std::string& params)
{
    BGN_TRY
    {
        m_duration.start();

        CHK_PARSE(m_reader.parse(params.c_str(), params.size()),
                string_utils::str_concat("Parse json error ", std::to_string(m_reader.get_parse_error()),
                                         ": ", m_reader.get_parse_error_str()).c_str());

        m_id = m_reader.get_id();
        m_writer.set_id(m_id);

        if (settings::system::validate_methods) {
            const rapidjson::SchemaDocument* schema = jsonrpc_schema::get()->get_schema(jsonrpc_schema::type::methods);
            CHK_REQ(schema, "Could not load validation schema");
            rapidjson::SchemaValidator validator(*schema);
            if (!m_reader.get_doc().Accept(validator)) {
                 LOGERR << "[" << get_name() << "] Json validation error for method " << m_reader.get_method();
                 m_writer.set_error(-32600, string_utils::str_concat("Json validation error for method ", m_reader.get_method()).c_str());
                 m_writer.set_error_data(rapidjson::Value(validator.GetError(), m_writer.get_allocator()));
                 return false;
            }
        }

        const bool complete = prepare_params();
        if (!complete && !m_result.first)
        {
            // prepare_params must set an error

            if (!m_writer.is_error())
            {
                m_writer.reset();
                m_writer.set_error(-32602, "Invalid params");
            }
        }
        LOGINFO << "[" << get_name() << "] Prepared json (complete = " << complete << ", async = " << m_result.first << ")";

#ifdef _DEBUG_
        LOGDEBUG << "Prepared json: \n" << m_writer.stringify();
#endif
        return complete;
    }
    END_TRY(return false)
}
