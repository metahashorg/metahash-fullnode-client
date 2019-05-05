#ifndef AUTO_CACHE_H
#define AUTO_CACHE_H

#include <thread>
#include <mutex>
#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"

class blocks_cache
{
public:
    blocks_cache();
    ~blocks_cache();

    bool start();
    void stop();

    bool runing() const;
    unsigned int next_block() const;

    bool get_block_by_num(unsigned int number, std::string& result);
    bool get_block_by_num(std::string& number, std::string& result);
    bool get_block_by_hash(std::string& hash, std::string& num, std::string& result);

protected:
    static void worker_proc(blocks_cache* param);

    void routine();
    void routine_2();
    bool save_block(unsigned int number, const std::string& dump);
    bool save_block(unsigned int number, const std::string_view& hash, const std::string_view& dump);
    bool update_number(unsigned int number);

private:
    bool        m_run;
    unsigned    m_nextblock;
    std::unique_ptr<std::thread> m_worker;
    std::unique_ptr<leveldb::DB> m_db;
    std::unique_ptr<leveldb::Cache> m_dbcache;
    std::unique_ptr<const leveldb::FilterPolicy> m_dbpolicy;
};

#endif // AUTO_CACHE_H
