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
    struct blk_info {
        blk_info(blk_number _number, const std::vector<unsigned char>& _hash): number(_number), hash(_hash) {}
        blk_number number;
        std::vector<unsigned char> hash;
        std::vector<std::vector<unsigned char>> extra;
    };

    enum ext_data_value: char {
        blk_not_checked,
        blk_not_signed,
        blk_awaiting_signature,
        blk_signed
    };

private:
    blocks_cache();

public:
//    static const blk_number extra_blocks_epoch;

    bool init();
    bool start();
    void stop();

    bool runing() const;
    blk_number next_block() const;
    blk_number last_signed_block() const;

    bool get_block_by_num(blk_number number, std::string& result) const;
    bool get_block_by_num(const std::string& number, std::string& result) const;
    bool get_block_by_hash(const std::string& hash, std::string& num, std::string& result) const;
    bool get_block_num_by_hash(const std::string& hash, std::string& result) const;
    bool get_extra_block_for(blk_number number, std::string& result) const;
    bool get_extra_data(blk_number number, std::string& result) const;
protected:
    static void worker_proc(blocks_cache* param);

    void routine();
    void routine_2();

    bool save_block(blk_number number, const std::string& hash, const std::string_view& dump, const std::string_view& ext_data);
    bool save_extra_block(const std::string& number, const std::string_view& dump);
    bool save_extra_data(const std::string& number, const std::string_view& dump);

    bool update_number(blk_number number);
    bool update_last_signed(blk_number number);

    int core_addr_verification(const torrent_node_lib::BlockInfo& bi, const std::string& prev_hash);
    int core_addr_verification(const torrent_node_lib::SignBlockInfo& bi);

    void dump_bad_block(size_t num, const char* buf, size_t size);

    bool auto_signed_block(blk_number number) const;

private:
    blk_number get_count_blocks();

private:
    bool                                            m_run;
    blk_number                                      m_nextblock;
    blk_number                                      m_last_signed_block;
    std::unique_ptr<std::thread>                    m_worker;
    std::unique_ptr<leveldb::DB>                    m_db;
    std::unique_ptr<leveldb::Cache>                 m_dbcache;
    std::unique_ptr<const leveldb::FilterPolicy>    m_dbpolicy;
};

#endif // BLOCKS_CACHE_H
