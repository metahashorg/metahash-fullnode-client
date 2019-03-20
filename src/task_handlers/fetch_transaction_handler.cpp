#include "fetch_transaction_handler.h"
#include "settings/settings.h"
#include "extensions/tracking_history.h"
#include "common/string_utils.h"
#include "common/convertStrings.h"

extern ext::tracking_history g_track_his;

fetch_transaction_handler::fetch_transaction_handler(http_session_ptr session)
    : base_handler(session)
{
    m_duration.set_message(__func__);
}

bool fetch_transaction_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        CHK_PRM(settings::extensions::use_tracking_history, "Tracking history is disabled");

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "address", m_addr), "address field not found")
        CHK_PRM(!m_addr.empty(), "address is empty")
        CHK_PRM(m_addr.compare(0, 2, "0x") == 0, "address field incorrect format")

        CHK_PRM(m_reader.get_value(*params, "data", m_data), "data field not found")
        CHK_PRM(!m_data.empty(), "data is empty")

        return true;
    }
    END_TRY_RET(false)
}

void fetch_transaction_handler::execute()
{
    BGN_TRY
    {
        m_writer.reset();
        std::string result;
        leveldb::Status status = g_track_his.get_history(m_addr, result);
        if (status.ok()) {

            rapidjson::Document doc;
            doc.Parse(result.c_str(), result.size());
            if (doc.GetParseError() != rapidjson::ParseErrorCode::kParseErrorNone) {
                m_writer.set_error(37621, string_utils::str_concat("parse history error: ", std::to_string(doc.GetParseError())));
                return;
            }

            m_data = common::toHex(m_data.begin(), m_data.end());

            rapidjson::Value arr(rapidjson::kArrayType);
            rapidjson::Value::MemberIterator data;
            for (auto& v : doc.GetArray()) {
                data = v.FindMember("data");
                if (data == v.MemberEnd()) {
                    // TODO Warning
                    continue;
                }
                std::string_view tmp(data->value.GetString(), data->value.GetStringLength());
                if (m_data.compare(tmp) == 0) {
                    arr.PushBack(v, m_writer.get_allocator());
                }
            }
            m_writer.set_result(arr);
        } else {
            m_writer.set_error(37621, string_utils::str_concat("geting history error: ", status.ToString()));
        }
    }
    END_TRY_RET();
}
