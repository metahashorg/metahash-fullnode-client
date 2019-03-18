#include "tracking.h"
#include "task_handlers/fetch_history_handler.h"
#include "common/string_utils.h"
#include "json_rpc.h"
#include "settings/settings.h"
#include "common/filesystem_utils.h"

namespace ext
{

tracking_history::tracking_history()
    : m_id("Tracking history")
    , m_run(false)
{
}

tracking_history::~tracking_history()
{
    stop();
}

bool tracking_history::init()
{
    EXT_BGN
    {
        if (!fs_utils::dir::is_exists(settings::extensions::tracking_history_folder.c_str())){
            if (!fs_utils::dir::create(settings::extensions::tracking_history_folder.c_str())) {
                EXT_ERR("Could not creat folder " << settings::extensions::tracking_history_folder);
                return false;
            };
        }
        return true;
    }
    EXT_END(false)
}

void tracking_history::run()
{
    EXT_BGN
    {
        if (m_run) {
            stop();
        }
        m_run = true;
        m_worker.reset(new std::thread(thread_proc, this));
    }
    EXT_END()
}

void tracking_history::stop()
{
    EXT_BGN
    {
        m_run = false;
        if (m_worker && m_worker->joinable()) {
            m_worker->join();
            m_worker.reset();
        }
    }
    EXT_END()
}

void tracking_history::get_history(const std::string_view& data, std::string& result)
{
    EXT_BGN
    {
    }
    EXT_END()
}

void tracking_history::thread_proc(tracking_history* param)
{
    param->routine();
}

void tracking_history::routine()
{
    EXT_BGN
    {
        EXT_INF("started");

        std::chrono::system_clock::time_point tp;
        json_rpc_reader reader;
        handler_result res;
        rapidjson::Value *tmp = nullptr;
        rapidjson::Value::ConstMemberIterator it;

        while (m_run) {
            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);

            for (auto& info: m_list_addr) {
                std::string json = string_utils::str_concat(
                    "{\"id\":1, \"version\":\"2.0\",\"method\":\"fetch-balance\", \"params\":{\"address\":\"", info.address, "\", \"beginTx\":", std::to_string(info.beginTx), ", \"countTxs\":10}}");
                res = perform<fetch_history_handler>(nullptr, json);
                if (res.message.empty()) {
                    EXT_ERR("Empty response (" << info.address << ")");
                    continue;
                }
                if (!reader.parse(res.message)) {
                    EXT_ERR("Parse response error " << reader.get_parse_error().Code() << " (" << info.address << ")");
                    continue;
                }
                tmp = reader.get_error();
                if (tmp) {
                    EXT_ERR("Response error (" << info.address << ") : " << reader.stringify(tmp));
                    continue;
                }
                tmp = reader.get_result();
                if (!tmp) {
                    EXT_ERR("Response without result (" << info.address << ")");
                    continue;
                }
                for (const auto& item: tmp->GetArray()) {
                    it = item.FindMember("data");
                    if (it == item.MemberEnd()) {
                        EXT_WRN("Data field not found (" << info.address << ")");
                        continue;
                    }
                    if (!it->value.IsString()) {
                        EXT_WRN("Data field has not string type (" << info.address << ")");
                        continue;
                    }
                    if (it->value.GetStringLength() == 0) {
                        // skip empty data
                        continue;
                    }

                    // TODO save transaction

                }
                info.beginTx += tmp->Size();
            }

            std::this_thread::sleep_until(tp);
        }

        EXT_INF("stoped");
    }
    EXT_END()
    m_run = false;
}

}
