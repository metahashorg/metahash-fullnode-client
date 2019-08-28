#include "fetch_history_filter_handler.h"
#include "settings/settings.h"
#include "utils.h"
#include "string_utils.h"

const std::vector<const char*> fetch_history_filter_handler::key_str = {
    "isInput",
    "isOutput",
    "isSuccess",
    "isForging",
    "isTest",
    "isDelegate"};

fetch_history_filter_handler::fetch_history_filter_handler(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx)
{
    m_duration.set_message(__func__);
    m_name = __func__;
}

bool fetch_history_filter_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "address", m_addr), "address field not found")
        CHK_PRM(!m_addr.empty(), "address is empty")
        CHK_PRM(utils::validate_address(m_addr), "address is invalid")

        auto filters = m_reader.get("filters", *params);
        CHK_PRM(filters, "filters field not found")

        bool found = false;
        rapidjson::Value res(rapidjson::Type::kObjectType);
        for (rapidjson::Value::ConstMemberIterator it = filters->MemberBegin(); it != filters->MemberEnd(); ++it) {
            found = false;
            for (const auto& v : key_str) {
                if (strlen(v) != it->name.GetStringLength()) {
                    continue;
                }
                if (strncmp(v, it->name.GetString(), it->name.GetStringLength()) != 0) {
                    continue;
                }
                found = true;
                break;
            }
            if (!found) {
                continue;
            }
            res.AddMember(const_cast<rapidjson::Value&>(it->name),
                          const_cast<rapidjson::Value&>(it->value), m_writer.get_allocator());
        }

        CHK_PRM(res.MemberBegin() != res.MemberEnd(), "filters is not recognized")

        m_writer.add_param("address", m_addr.c_str());
        m_writer.get_params()->AddMember("filters", res, m_writer.get_allocator());

        return true;
    }
    END_TRY(return false)
}

//void fetch_history_filter_handler::execute()
//{
//    BGN_TRY
//    {
//        if (settings::system::useLocalDatabase) {
//            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
//            const torrent_node_lib::Sync &sync = *syncSingleton();
//            const std::vector<torrent_node_lib::TransactionInfo> txs = sync.getTxsForAddress(torrent_node_lib::Address(m_addr), m_beginTx, m_countTxs, m_countTxs);
//            addressesInfoToJson(m_addr, txs, sync.getBlockchain(), 0, false, JsonVersion::V1, m_writer.get_doc());
//        } else {
//            base_network_handler::execute();
//        }
//    }
//    END_TRY();
//}
