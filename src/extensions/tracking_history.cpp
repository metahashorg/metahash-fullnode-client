#include "tracking_history.h"
#include "task_handlers/fetch_history_handler.h"
#include "common/string_utils.h"
#include "json_rpc.h"
#include "settings/settings.h"
#include "common/filesystem_utils.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "http_server.h"

extern std::unique_ptr<http_server> g_server;

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
        std::string folder = settings::extensions::tracking_history_folder;
        if (folder[folder.size()-1] != '/') {
            folder.append("/");
        }
        m_file.append(folder);
        m_file.append("addresses");
        std::fstream fs;
        fs.open(m_file.c_str());
        if (!fs.is_open()) {
            fs.open(m_file.c_str(), std::fstream::out);
            if (!fs.is_open()) {
                EXT_ERR("Could not open addresses file " << m_file << " : " << strerror(errno));
                return false;
            }
            fs << "{\"list\":[{\"address\":\"\",\"last_known\":\"\"}]}";
            fs.flush();
            fs.close();
            fs.open(m_file.c_str());
            if (!fs.is_open()) {
                EXT_ERR("Could not open addresses file " << m_file << " : " << strerror(errno));
                return false;
            }
        }
        rapidjson::Document doc;
        rapidjson::IStreamWrapper isw(fs);
        doc.ParseStream(isw);
        fs.close();
        if (doc.HasParseError()) {
            EXT_ERR("Parse addresses file error: " << doc.GetParseError());
            return false;
        }
        auto list = doc.FindMember("list");
        if (list == doc.MemberEnd()) {
            EXT_ERR("Addresses file has incorrect format: missing 'list' field");
            return false;
        }

        // database init

        std::string db_loc(folder + "data/");
        if (!fs_utils::dir::is_exists(db_loc.c_str())) {
            if (!fs_utils::dir::create(db_loc.c_str())) {
                EXT_ERR("Could not create data storage path");
                return false;
            }
        }

        leveldb::Options options;
        options.create_if_missing = true;

        leveldb::DB* db = nullptr;
        auto status = leveldb::DB::Open(options, db_loc.c_str(), &db);
        if (!status.ok()) {
            EXT_ERR("Could not open database: " << status.ToString().c_str());
            return false;
        }
        m_db.reset(db);

        // list addresses init

        m_list_addr.clear();
        rapidjson::Value::ConstMemberIterator it;
        for (const auto& info: list->value.GetArray()) {
            addr_info ai;
            it = info.FindMember("address");
            if (it == info.MemberEnd()) {
                EXT_WRN("Could not find 'address' field");
                continue;
            }
            ai.address = it->value.GetString();
            if (ai.address.empty()) {
                continue;
            }
            std::transform(ai.address.begin(), ai.address.end(), ai.address.begin(), ::tolower);

            if (std::find_if(m_list_addr.begin(), m_list_addr.end(), [&ai](addr_info& v){
                return v.address == ai.address;
            }) != m_list_addr.end()) {
                EXT_WRN("Duplicate address " << ai.address << " found");
                continue;
            }

            it = info.FindMember("last_known");
            if (it == info.MemberEnd()) {
                EXT_WRN("Could not find 'last_known' field");
                continue;
            }
            ai.last_known = it->value.GetString();
            m_list_addr.push_back(ai);
        }
        EXT_INF("Address list size: " << m_list_addr.size());
        if (m_list_addr.empty()) {
            EXT_WRN("Address list is empty. Canceled.");
            return false;
        }

        EXT_INF("Init successfully");
        return true;
    }
    EXT_END(false)
}

void tracking_history::update_list()
{
    EXT_BGN
    {
        std::fstream fs;
        fs.open(m_file.c_str());
        if (!fs.is_open()) {
            EXT_ERR("Could not open addresses file " << m_file << " : " << strerror(errno));
            return;
        }
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Value arr(rapidjson::kArrayType);

        for (const auto& v: m_list_addr){
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("address", v.address, doc.GetAllocator());
            obj.AddMember("last_known", v.last_known, doc.GetAllocator());
            arr.PushBack(obj, doc.GetAllocator());
        }
        doc.AddMember("list", arr, doc.GetAllocator());

        rapidjson::StringBuffer buf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        doc.Accept(writer);
        fs.write(buf.GetString(), buf.GetLength());
        fs.flush();
        fs.close();
    }
    EXT_END()
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

leveldb::Status tracking_history::get_history(const std::string& address, std::string& result)
{
    EXT_BGN
    {
        if (!m_db) {
            return leveldb::Status::Corruption("dabase not initialized");
        }
        if (std::find_if(m_list_addr.begin(), m_list_addr.end(), [&address](addr_info& v){
            return v.address == address;
        }) == m_list_addr.end()) {
            EXT_WRN("Address " << address << " not found");
            return leveldb::Status::NotFound(address);
        }
        leveldb::ReadOptions opt;
        return m_db->Get(opt, address, &result);
    }
    EXT_END(leveldb::Status::Corruption("failed on getting history"))
}

bool tracking_history::put_history(const std::string& address, const rapidjson::Value& data)
{
    EXT_BGN
    {
        if (!m_db) {
            return false;
        }
        std::string result;
        leveldb::ReadOptions opt;
        leveldb::Status status = m_db->Get(opt, address, &result);
        if (status.IsNotFound()) {
            result = "[]";
            status = leveldb::Status::OK();
        }
        if (status.ok()) {

            rapidjson::Document doc;
            doc.Parse(result.c_str(), result.size());
            if (doc.GetParseError() != rapidjson::ParseErrorCode::kParseErrorNone) {
                EXT_ERR("Could not parse DB result for " << address << " : " << doc.GetParseError());
                return false;
            }

            rapidjson::Value::ConstMemberIterator tr = data.FindMember("transaction");
            std::string_view tr1(tr->value.GetString(), tr->value.GetStringLength());

            rapidjson::Value::ConstMemberIterator it;
            for (const auto& v: doc.GetArray()) {
                it = v.FindMember("transaction");
                std::string_view tr2(it->value.GetString(), it->value.GetStringLength());
                if (tr2.compare(tr1) == 0) {
                    EXT_WRN("Transaction duplicat " << tr1 << " from address " << address);
                    return true;
                }
            }

            rapidjson::Value val;
            val.CopyFrom(data, doc.GetAllocator());
            doc.PushBack(val, doc.GetAllocator());
            rapidjson::StringBuffer buf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
            doc.Accept(writer);

            leveldb::WriteOptions opt;
            status = m_db->Put(opt, address, leveldb::Slice(buf.GetString(), buf.GetLength()));
            if (status.ok()) {
                EXT_INF("Saved transaction " << tr1.data() << " from address " << address);
                return true;
            }
            EXT_ERR("Could not save transaction for " << address << " : " << status.ToString());
        } else {
            EXT_ERR("Could not get transaction for " << address << " : " << status.ToString());
        }
        return false;
    }
    EXT_END(false)
}

void tracking_history::thread_proc(tracking_history* param)
{
    param->routine();
}

void tracking_history::routine()
{
    EXT_BGN
    {
        EXT_INF("Started");

        std::chrono::system_clock::time_point tp;
        json_rpc_reader reader;
        handler_result res;
        rapidjson::Value *tmp = nullptr;
        rapidjson::Value::ConstMemberIterator it, trnz;
        bool need_update = false;

        while (m_run) {
            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);

            if (!g_server || !g_server->runnig()){
                goto wait;
            }

            for (auto& info: m_list_addr) {

                uint64_t begin_tx = 0;
                bool cancel = false;
                while (!cancel) {

                    std::string json = string_utils::str_concat(
                        "{\"id\":1, \"version\":\"2.0\",\"method\":\"fetch-history\", \"params\":{\"address\":\"", info.address, "\", \"beginTx\":", std::to_string(begin_tx),",\"countTxs\":10}}");
                    res = perform<fetch_history_handler>(nullptr, json);

                    if (res.message.empty()) {
                        EXT_ERR("Empty response (" << info.address << ")");
                        break;
                    }

                    if (!reader.parse(res.message)) {
                        EXT_ERR("Parse response error " << reader.get_parse_error().Code() << " (" << info.address << ")");
                        break;
                    }

                    tmp = reader.get_error();
                    if (tmp) {
                        EXT_ERR("Response error (" << info.address << ") : " << reader.stringify(tmp));
                        break;
                    }

                    tmp = reader.get_result();
                    if (!tmp) {
                        EXT_ERR("Response without result (" << info.address << ")");
                        break;
                    }

                    if (!tmp->IsArray() || tmp->Size() == 0) {
                        break;
                    }

                    for (const auto& item: tmp->GetArray()) {

                        trnz = item.FindMember("transaction");
                        if (info.last_known.compare(trnz->value.GetString()) == 0) {
                            cancel = true;
                            break;
                        }

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

                        if (!put_history(info.address, item)) {
                            EXT_ERR("Could not save transaction " << it->value.GetString() << " from address " << info.address);
                            // TODO break
                            continue;
                        }
    //                    EXT_INF("Saved transaction " << it->value.GetString() << " from address " << info.address);
                    }

                    if (begin_tx == 0 && tmp->Size() > 0) {
                        trnz = tmp->GetArray().Begin()->FindMember("transaction");
                        if (info.last_known.compare(trnz->value.GetString()) != 0) {
                            info.last_known = trnz->value.GetString();
                            need_update = true;
                        }
                    }

                    if (cancel) {
                        break;
                    }

                    begin_tx += tmp->Size();
                }
            }
            if (need_update) {
                need_update = false;
                update_list();
            }

wait:
            std::this_thread::sleep_until(tp);
        }

        EXT_INF("Stoped");
    }
    EXT_END()
    m_run = false;
}

}
