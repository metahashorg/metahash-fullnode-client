#pragma once
#include <map>
#include <mutex>

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
}
