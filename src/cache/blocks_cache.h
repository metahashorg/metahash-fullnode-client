#ifndef AUTO_CACHE_H
#define AUTO_CACHE_H

#include <thread>
#include <mutex>
#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"

namespace torrent_node_lib {
    struct BlockInfo;
}

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
    bool save_block(unsigned int number, const std::string& hash, const std::string_view& dump);
    bool update_number(unsigned int number);

    bool core_addr_verification(const torrent_node_lib::BlockInfo& bi, const std::string& prev_hash);
    void dump_bad_block(size_t num, const char* buf, size_t size);

private:
    bool        m_run;
    unsigned    m_nextblock;
    std::unique_ptr<std::thread> m_worker;
    std::unique_ptr<leveldb::DB> m_db;
    std::unique_ptr<leveldb::Cache> m_dbcache;
    std::unique_ptr<const leveldb::FilterPolicy> m_dbpolicy;
};

#endif // AUTO_CACHE_H
