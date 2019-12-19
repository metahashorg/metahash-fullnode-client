#ifndef BLOCKS_CACHE_H
#define BLOCKS_CACHE_H

#include <thread>
#include <mutex>
#include <vector>
#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"
#include "singleton.h"

namespace torrent_node_lib {
    struct BlockInfo;
    struct SignBlockInfo;
}

class blocks_cache: public singleton<blocks_cache>
{
    friend class singleton<blocks_cache>;

    using blk_number = std::size_t;
    using ext_blk_data = char[8];
    using blk_info = std::tuple<const blk_number, std::vector<unsigned char>>;

    enum ext_data_value: char {
        blk_not_checked,
        blk_not_signed,
        blk_signed
    };

private:
    blocks_cache();

public:
    bool init();
    bool start();
    void stop();

    bool runing() const;
    blk_number next_block() const;

    bool get_block_by_num(blk_number number, std::string& result);
    bool get_block_by_num(const std::string& number, std::string& result);
    bool get_block_by_hash(const std::string& hash, std::string& num, std::string& result);
    bool get_block_num_by_hash(const std::string& hash, std::string& result);
    bool get_extra_block_for(blk_number number, std::string& result);
protected:
    static void worker_proc(blocks_cache* param);

    void routine();
    void routine_2();

    bool save_block(blk_number number, const std::string& hash, const std::string_view& dump, const std::string_view& ext_data);
    bool save_extra_block(const std::string& number, const std::string_view& dump);
    bool save_extra_data(const std::string& number, const std::string_view& dump);

    bool update_number(blk_number number);

    bool core_addr_verification(const torrent_node_lib::BlockInfo& bi, const std::string& prev_hash);
    bool core_addr_verification(const torrent_node_lib::SignBlockInfo& bi);

    void dump_bad_block(size_t num, const char* buf, size_t size);

private:
    blk_number get_count_blocks();

private:
    bool                                            m_run;
    blk_number                                      m_nextblock;
    std::unique_ptr<std::thread>                    m_worker;
    std::unique_ptr<leveldb::DB>                    m_db;
    std::unique_ptr<leveldb::Cache>                 m_dbcache;
    std::unique_ptr<const leveldb::FilterPolicy>    m_dbpolicy;
};

#endif // BLOCKS_CACHE_H
