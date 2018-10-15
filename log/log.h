#pragma once
#include <map>
#include <mutex>

#define USE_DEBUG_LOG_MESSAGE 1

namespace logg
{
    class logger
    {
        static std::map<int, std::string> pri_str;
    public:
        logger();
        ~logger();

        logger(const logger&)               = delete;
        logger& operator =(const logger&)   = delete;

        static void push(int pri, const std::string& mess);

    protected:
        static std::mutex _locker;
    };

    static logger _logger;

    void push_err(const std::string& mess);
    void push_wrn(const std::string& mess);
    void push_inf(const std::string& mess);
    void push_dbg(const std::string& mess);
}

#define STREAM_LOG_BASE(v)\
    std::ostringstream stream;\
    stream << std::this_thread::get_id() << " " << v <<std::endl;\
    stream.flush();

#define STREAM_LOG_INF(v)\
    {STREAM_LOG_BASE(v)\
    logg::push_inf(stream.str());}

#define STREAM_LOG_ERR(v)\
    {STREAM_LOG_BASE(v)\
    logg::push_err(stream.str());}

#define STREAM_LOG_WRN(v)\
    {STREAM_LOG_BASE(v)\
    logg::push_wrn(stream.str());}

#define STREAM_LOG_DBG(v)\
    {STREAM_LOG_BASE(v)\
    logg::push_dbg(stream.str());}
