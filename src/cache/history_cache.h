#ifndef __HISTORY_CACHE_H_
#define __HISTORY_CACHE_H_

#define RAPIDJSON_HAS_STDSTRING 1

#include <thread>
#include <mutex>
#include <vector>
#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"
#include "rapidjson/document.h"

class history_cache
{
    const char* folder = "./data/hst_cache/";

    struct info {
        info(const std::string& address): begin(0), tx_count(0), addr(address) {
            trx.SetArray();
        }
        unsigned int begin;
        unsigned int tx_count;
        std::string addr;
        rapidjson::Document trx;
    };

public:
    history_cache();
    ~history_cache();

    bool start();
    void stop();

    bool runing() const;

    bool get_history(const std::string& address, rapidjson::Document& doc);

protected:
    static void worker_proc(history_cache* param);

//    rapidjson::Value* parse_response(json_response_type* response);

    void routine();
    bool save_transaction(const char* name, const rapidjson::Value& trx);
    bool save_history(const info& addr);

private:
    bool                            m_run;
    std::vector<info>               m_addrs;
    std::unique_ptr<std::thread>    m_worker;
    std::unique_ptr<leveldb::DB>    m_db;
    std::unique_ptr<leveldb::Cache> m_dbcache;
    std::unique_ptr<const leveldb::FilterPolicy> m_dbpolicy;
};

#endif // __HISTORY_CACHE_H_
