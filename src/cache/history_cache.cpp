#include "history_cache.h"
#include "common/stopProgram.h"
#include "log.h"
#include "settings/settings.h"
#include "common/filesystem_utils.h"
#include "common/string_utils.h"
#include "http_json_rpc_request.h"
#include "json_rpc.h"
#include <boost/exception/all.hpp>
#include <boost/asio/io_context.hpp>
#include "rapidjson/prettywriter.h"

#include "task_handlers/fetch_history_handler.h"
#include "task_handlers/fetch_balance_handler.h"

#define CACHE_BGN try

#define CACHE_END(ret) \
    catch (const common::StopException&) {\
        ret;\
    } catch (boost::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);\
        ret;\
    } catch (std::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();\
        ret;\
    } catch (...) {\
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";\
        ret;\
    }

history_cache::history_cache()
    : m_run(false)
{
}

history_cache::~history_cache()
{
}

bool history_cache::start()
{
    CACHE_BGN
    {
        if (!fs_utils::dir::is_exists("./data/")){
            if (!fs_utils::dir::create("./data/")) {
                LOGERR << "History cache. Could not create folder ./data/";
                return false;
            };
        }
        if (!fs_utils::dir::is_exists(history_cache::folder)){
            if (!fs_utils::dir::create(history_cache::folder)) {
                LOGERR << "History cache. Could not create folder " << history_cache::folder;
                return false;
            };
        }

        m_dbcache.reset(leveldb::NewLRUCache(500*1024*1024));
        m_dbpolicy.reset(leveldb::NewBloomFilterPolicy(10));

        leveldb::Options options;
        options.create_if_missing = true;
        options.block_cache = m_dbcache.get();
        options.filter_policy = m_dbpolicy.get();

        leveldb::DB* db = nullptr;
        leveldb::Status status = leveldb::DB::Open(options, history_cache::folder, &db);
        if (!status.ok()) {
            LOGERR << "History cache. Could not open database. " << status.ToString().c_str();
            return false;
        }
        m_db.reset(db);

        for (const auto& v: settings::system::history_cache_addrs) {
            if (!utils::validate_address(v)) {
                continue;
            }
            m_addrs.emplace_back(info(v));
        }

        LOGINFO << "History cache. Addresses count " << m_addrs.size();

        if (m_addrs.empty()) {
            return false;
        }

        leveldb::ReadOptions opt;
        rapidjson::ParseResult parse_res;
        std::string res;
        for (auto& v: m_addrs) {
            res.clear();
            status = m_db->Get(opt, v.addr, &res);
            if (!status.ok()) {
                continue;
            }
            parse_res = v.trx.Parse(res);
            if (parse_res.IsError()) {
                v.trx.SetNull();
                continue;
            }
            v.tx_count = v.trx.Size();
        }

        LOGINFO << "History cache. Init successfully";

        m_worker = std::make_unique<std::thread>(worker_proc, this);
        return true;
    }
    CACHE_END(return false)
}

void history_cache::stop()
{
    CACHE_BGN
    {
        m_run = false;
        if (m_worker.get() && m_worker->joinable()) {
            m_worker->join();
        }
    }
    CACHE_END()
}

bool history_cache::runing() const
{
    return m_run;
}

void history_cache::worker_proc(history_cache *param)
{
    param->routine();
}

void history_cache::routine()
{
    CACHE_BGN
    {
        LOGINFO << "History cache. Version 1 Started";
        m_run = true;

        handler_result res;
        std::string json;

        json_rpc_reader reader;
        const rapidjson::Value* tmp;
        rapidjson::Value::ConstMemberIterator it;
        rapidjson::Value::ConstValueIterator arr_it;
        std::chrono::system_clock::time_point tp;

        const json_response_type* response = nullptr;

        http_json_rpc_request_ptr fetch_history = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        fetch_history->set_path("fetch-history");

        http_json_rpc_request_ptr fetch_balance = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        fetch_balance->set_path("fetch-balance");

        while (m_run) {

            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(30);

            for (auto& v: m_addrs) {

                common::checkStopSignal();

                json.clear();
                string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\",\"method\":\"fetch-balance\", \"params\":{\"address\":\"", v.addr, "\"}}");

                fetch_balance->set_host(settings::server::get_tor().c_str());
                fetch_balance->set_body(json.c_str());
                fetch_balance->reset_attempts();
                fetch_balance->execute();
                response = fetch_balance->get_response();
                if (!response) {
                    LOGERR << "History cache. Could not get response from fetch-balance (" << v.addr << ")";
                    continue;
                }
                if (response->get().body().empty()) {
                    LOGERR << "History cache. Could not get fetch-balance (" << v.addr << ")";
                    continue;
                }
                if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                    LOGERR << "History cache. Could not parse fetch-balance (" << v.addr << "): " << reader.get_parse_error() << " " << reader.get_parse_error_str();
                    continue;
                }
                tmp = reader.get_error();
                if (tmp != nullptr) {
                    LOGERR << "History cache. Got error from fetch-balance ("  << v.addr << "): " << reader.stringify(tmp);
                    continue;
                }
                tmp = reader.get_result();
                if (tmp == nullptr) {
                    LOGERR << "History cache. Did not find result in fetch-balance (" << v.addr << ")";
                    continue;
                }
                it = tmp->FindMember("count_txs");
                if (it == tmp->MemberEnd()) {
                    LOGERR << "History cache. Did not find count_txs in fetch-balance (" << v.addr << ")";
                    continue;
                }
                if (v.trx.Size() == it->value.GetUint()) {
                    // nothing to update
                    continue;
                }
                v.tx_count = it->value.GetUint();
                v.trx.Reserve(v.tx_count, v.trx.GetAllocator());
                v.begin = 0;

                // go through all trx, not effective way
                // because fetch-history has result order transaction starting from last to first
                while (v.begin < v.tx_count && v.trx.Size() < v.tx_count) {
                    common::checkStopSignal();

                    json.clear();
                    string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\",\"method\":\"fetch-history\", \"params\":{\"address\":\"", v.addr, "\", \"beginTx\":", std::to_string(v.begin),",\"countTxs\":10}}");

                    fetch_history->set_host(settings::server::get_tor().c_str());
                    fetch_history->set_body(json.c_str());
                    fetch_history->reset_attempts();
                    fetch_history->execute();
                    response = fetch_history->get_response();
                    if (!response) {
                        LOGERR << "History cache. Could not get response from fetch-history (" << v.addr << ")";
                        continue;
                    }
                    if (response->get().body().empty()) {
                        LOGERR << "History cache. Could not get fetch-history (" << v.addr << ")";
                        continue;
                    }
                    if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                        LOGERR << "History cache. Could not parse fetch-history (" << v.addr << "): " << reader.get_parse_error() << " " << reader.get_parse_error_str();
                        continue;
                    }
                    tmp = reader.get_error();
                    if (tmp != nullptr) {
                        LOGERR << "History cache. Got error from fetch-history ("  << v.addr << "): " << reader.stringify(tmp);
                        continue;
                    }
                    tmp = reader.get_result();
                    if (tmp == nullptr) {
                        LOGERR << "History cache. Did not find result in fetch-history (" << v.addr << ")";
                        continue;
                    }
                    if (!tmp->IsArray()) {
                        LOGERR << "History cache. Expect array from fetch-history (" << v.addr << ")";
                        continue;
                    }
                    if (tmp->Size() == 0) {
                        v.begin = 0;
                        continue;
                    }

                    for (arr_it = tmp->Begin(); arr_it != tmp->End(); arr_it++) {
                        it = arr_it->FindMember("transaction");
                        if (it == arr_it->MemberEnd()) {
                            LOGERR << "History cache. Could not find transaction field ("  << v.addr << ")";
                            continue;
                        }
                        bool found = false;
                        for (const auto& t: v.trx.GetArray()) {
                            if (strncmp(t.GetString(), it->value.GetString(), t.GetStringLength()) == 0) {
                                found = true;
                                break;
                            }
                        }
                        if (found) {
                            continue;
                        }
                        if (!save_transaction(it->value.GetString(), *arr_it)) {
                            LOGERR << "History cache. Could not save transaction ("  << v.addr << ")";
                            continue;
                        }
                        v.trx.PushBack(rapidjson::Value(it->value.GetString(), it->value.GetStringLength(), v.trx.GetAllocator()).Move(), v.trx.GetAllocator());
                        LOGINFO << "History cache. Transaction " << it->value.GetString() << " for address "  << v.addr << " has been saved.";
                    }
                    v.begin += 10;
                }
                if (!save_history(v)) {
                    LOGERR << "History cache. Could not save history ("  << v.addr << ")";
                }
            }
            common::checkStopSignal();
            std::this_thread::sleep_until(tp);
        }
    }
    CACHE_END()
    m_run = false;
    LOGINFO << "History cache. Stoped";
}

bool history_cache::save_transaction(const char* name, const rapidjson::Value& trx)
{
    CACHE_BGN
    {
        if (!m_db) {
            LOGERR << "History cache. Database is not initialized";
            return false;
        }

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        trx.Accept(writer);

        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, name, leveldb::Slice(buf.GetString(), buf.GetLength()));

        return status.ok();
    }
    CACHE_END(return false)
}

bool history_cache::save_history(const info& inf)
{
    CACHE_BGN
    {
        if (!m_db) {
            LOGERR << "History cache. Database is not initialized";
            return false;
        }

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
        inf.trx.Accept(writer);

        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, inf.addr, leveldb::Slice(buf.GetString(), buf.GetLength()));

        return status.ok();
    }
    CACHE_END(return false)
}

bool history_cache::get_history(const std::string& address, rapidjson::Document& doc)
{
    CACHE_BGN
    {
        bool found = false;
        std::string addr = string_utils::to_lower(address);
        for (const auto& v: m_addrs) {
            if (addr.compare(v.addr) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LOGWARN << "History cache. Get history. Address " << address << " is not found";
            doc.SetString("Address is not found");
            return false;
        }
        if (!m_db) {
            LOGERR << "History cache. Get history. Database is not initialized";
            doc.SetString("Database is not initialized");
            return false;
        }
        std::string str;
        leveldb::ReadOptions opt;
        leveldb::Status status = m_db->Get(opt, addr, &str);
        if (!status.ok()) {
            LOGERR << "History cache. Get history. Got database error " << status.ToString();
            doc.SetString(string_utils::str_concat("Database error ", status.ToString()), doc.GetAllocator());
            return false;
        }
        rapidjson::Document hst;
        rapidjson::ParseResult parse_res = hst.Parse(str);
        if (parse_res.IsError()) {
            LOGERR << "History cache. Get history. Parse error " << parse_res.Code();
            doc.SetString(string_utils::str_concat("History parse error ", std::to_string(parse_res.Code())), doc.GetAllocator());
            return false;
        }
        if (!hst.IsArray()) {
            LOGERR << "History cache. Get history. History has different format " << hst.GetType();
            doc.SetString(string_utils::str_concat("History has different format ", std::to_string(hst.GetType())), doc.GetAllocator());
            return false;

        }
        rapidjson::Document t;
        doc.SetArray();
        doc.Reserve(hst.Size(), doc.GetAllocator());
        for (const auto& trx: hst.GetArray()) {
            str.clear();
            status = m_db->Get(opt, trx.GetString(), &str);
            if (!status.ok()) {
                LOGERR << "History cache. Get history. Could not find transaction (" << trx.GetString() << ") for address " << address;
                continue;
            }
            if (str.empty()) {
                LOGERR << "History cache. Get history. Empty transaction (" << trx.GetString() << ") record for address " << address;
                continue;
            }
            parse_res = t.Parse(str);
            if (parse_res.IsError()) {
                LOGERR << "History cache. Get history. Transaction (" << trx.GetString() << ") parse error " << parse_res.Code();
                continue;
            }
            doc.PushBack(rapidjson::Value().CopyFrom(t, doc.GetAllocator()), doc.GetAllocator());
        }
        return true;
    }
    CACHE_END(return false)
}
