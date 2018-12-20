#pragma once

#include <string.h>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace utils
{
    template <typename T>
    void write_compact_int(T value, std::vector<unsigned char>& buf)
    {
        size_t sz = sizeof(T);
        unsigned char* p = (unsigned char*)(&value) + (sz - 1);
        while (sz)
        {
            if (*p--) break;
            sz--;
        }
        if (value == 0)
            sz = 1;

        size_t type = value >= 0xfa ? 1 : 0;
        size_t pos = buf.size();
        buf.resize(pos + sz + type);
        if (type)
        {
            if (sz <= 2)
                buf[pos++] = 0xfa;
            else if (sz <= 4)
                buf[pos++] = 0xfb;
            else if (sz <= 8)
                buf[pos++] = 0xfc;
            else if (sz <= 16)
                buf[pos++] = 0xfd;
            else if (sz <= 32)
                buf[pos++] = 0xfe;
            else if (sz <= 64)
                buf[pos++] = 0xff;
            else
                throw std::invalid_argument("write_compact_int");
        }
        memcpy(&buf[pos], &value, sz);
    }

    void parse_address(const std::string& address, std::string& host, std::string& port, std::string& path, bool& use_ssl);

    bool gen_sign(std::string &transaction, std::string& result, const std::string& prv_key, const char* fmt, ...);

    class Timer
    {
    public:
        typedef std::chrono::milliseconds Interval;
        typedef std::function<void(void)> Handler;

        Timer() {};
        ~Timer();

        void start(const Interval& interval, const Handler& handler, bool immediately = true);
        void stop();
        void run_once();

    private:
        std::thread         m_thr;
        Handler             m_handler;
        std::condition_variable cond;
        std::mutex          m_locker;
        Interval            m_interval;
        bool isStopped = true;
    };
}
