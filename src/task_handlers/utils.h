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
    template<typename Integer>
    inline std::string toLittleEndian(Integer integer) {
        std::array<unsigned char, sizeof(integer)> arr;
        for (size_t i = 0; i < arr.size(); i++) {
            arr[i] = integer % 256;
            integer /= 256;
        }
        return std::string(arr.begin(), arr.end());
    }
    
    inline std::vector<unsigned char> toVect(const std::string &str) {
        return std::vector<unsigned char>(str.begin(), str.end());
    }
    
    template <typename T>
    void write_compact_int(T value, std::vector<unsigned char>& buf) {
        std::vector<unsigned char> result;
        if (value <= 249) {
            result = toVect(toLittleEndian(uint8_t(value)));
        } else if (value <= std::numeric_limits<uint16_t>::max()) {
            result = toVect(toLittleEndian(uint8_t(250)) + toLittleEndian(uint16_t(value)));
        } else if (value <= std::numeric_limits<uint32_t>::max()) {
            result = toVect(toLittleEndian(uint8_t(251)) + toLittleEndian(uint32_t(value)));
        } else {
            result = toVect(toLittleEndian(uint8_t(252)) + toLittleEndian(uint64_t(value)));
        }
        buf.insert(buf.end(), result.begin(), result.end());
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
