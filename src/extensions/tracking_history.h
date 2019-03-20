#ifndef __TRACKING_HISTORY_H__
#define __TRACKING_HISTORY_H__

#define RAPIDJSON_HAS_STDSTRING 1

#include <string>
#include <thread>
#include <memory>
#include <vector>
#include "log.h"
#include "leveldb/db.h"
#include "rapidjson/document.h"

namespace ext
{

class tracking_history
{
    struct addr_info {
        std::string address;
        size_t beginTx = {0};
    };

public:
    tracking_history();
    ~tracking_history();

    bool init();
    void run();
    void stop();

    leveldb::Status get_history(const std::string& address, std::string& result);

protected:
    static void thread_proc(tracking_history* param);

    void update_list();

    bool put_history(const std::string& address, rapidjson::Value& data);
    void routine();

protected:
    std::string m_id;
    std::string m_file;
    std::unique_ptr<std::thread> m_worker;
    std::vector<addr_info> m_list_addr;
    std::unique_ptr<leveldb::DB> m_db;
    bool m_run;
};

#define EXT_ERR(msg) LOGERR << "[" << m_id << "] " << msg;
#define EXT_WRN(msg) LOGWARN << "[" << m_id << "] " << msg;
#define EXT_INF(msg) LOGINFO << "[" << m_id << "] " << msg;

#define EXT_BGN try

#define EXT_END(ret) \
catch (std::exception& ex) { \
    EXT_ERR("std::exception " << ex.what()) \
    return ret;\
} catch (...) {\
    EXT_ERR("Unknown exception")\
    return ret;\
}

}

#endif // __TRACKING_HISTORY_H__
