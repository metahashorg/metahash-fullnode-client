#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <string>
#include <thread>
#include <memory>
#include <vector>
#include "log.h"

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

    void get_history(const std::string_view& data, std::string& result);

protected:
    static void thread_proc(tracking_history* param);

    void routine();

protected:
    std::string m_id;
    std::unique_ptr<std::thread> m_worker;
    std::vector<addr_info> m_list_addr;
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

#endif // __TRACKING_H__
