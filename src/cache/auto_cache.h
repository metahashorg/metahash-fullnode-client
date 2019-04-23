#ifndef AUTO_CACHE_H
#define AUTO_CACHE_H

#include <thread>
#include <mutex>
#include "leveldb/db.h"

class auto_cache
{
public:
    auto_cache();
    ~auto_cache();

    bool start();
    void stop();
    bool running() const;

protected:
    static void worker_proc(auto_cache* param);

    void routine();
    void save_block(unsigned int number, const std::string_view& dump);

private:
    bool        m_run;
    unsigned    m_nextblock;
    std::unique_ptr<std::thread> m_worker;
    std::unique_ptr<leveldb::DB> m_db;
    std::mutex  m_locker;
};

#endif // AUTO_CACHE_H
