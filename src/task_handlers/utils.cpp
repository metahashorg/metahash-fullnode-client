#include <stdarg.h>
#include "utils.h"
#include "../cpplib_open_ssl_decor/crypto.h"
#include <iostream>
#include "time_duration.h"

#include "common/log.h"

namespace utils
{
    void parse_address(const std::string& address, std::string& host, std::string& port, std::string& path, bool& use_ssl)
    {
        std::string_view tmp = address;

        auto pos = tmp.find("http://");
        if (pos != std::string::npos)
        {
            port.clear();
            port = "80";
            tmp.remove_prefix(7);
        }

        pos = tmp.find("https://");
        if (pos != std::string::npos)
        {
            port.clear();
            port = "443";
            tmp.remove_prefix(8);
            use_ssl = true;
        }

        pos = tmp.find("/");
        if (pos != std::string::npos)
        {
            path.clear();
            path = tmp.substr(pos);
            tmp.remove_suffix(tmp.size() - pos);
        }

        pos = tmp.find(":");
        if (pos != std::string::npos)
        {
            port.clear();
            port = tmp.substr(pos + 1);
            tmp.remove_suffix(tmp.size() - pos);
        }

        host = tmp;
    }

    bool gen_sign(std::string &transaction, std::string& result, const std::string& prv_key, const char* fmt, ...)
    {
        try
        {
            std::vector<unsigned char> data;
            va_list args;
            va_start(args, fmt);
            while (*fmt != '\0')
            {
                switch(*fmt)
                {
                case 'd':
                    utils::write_compact_int(va_arg(args, uint32_t), data);
                    break;
                case 'D':
                    utils::write_compact_int(va_arg(args, uint64_t), data);
                    break;
                case 'x':
                {
                    std::string param = va_arg(args, char*);
                    if (param.size() > 2)
                    {
                        if (param[0] == '0' && param[1] == 'x')
                            param.erase(0, 2);
                    }
                    auto bin = hex2bin(param);
                    data.insert(std::end(data), std::begin(bin), std::end(bin));
                    break;
                }
                case 's':
                {
                    std::string param = va_arg(args, char*);
                    data.insert(std::end(data), std::begin(param), std::end(param));
                    break;
                }
                default:
                    break;
                }
                ++fmt;
            }
            va_end(args);
            if (data.empty())
                return false;

            std::vector<unsigned char> sign;
            CRYPTO_sign_data(sign, prv_key, data);

            result = bin2hex(sign);
            transaction = bin2hex(data);
            return true;
        }
        catch (std::exception& e)
        {
            LOGERR << "generate sign failed: " << e.what();
            result = "generate sign failed: ";
            result.append(e.what());
            return false;
        }
    }

    // Timer
    Timer::~Timer() {
        stop();
    }

    void Timer::start(const Interval& interval, const Handler& handler, bool immediately /*= true*/) {
        m_handler = handler;
        m_interval = interval;
        if (immediately) {
            isStopped = false;
            run_once();
        } else {
            isStopped = true;
        }
    }

    void Timer::stop() {
        if (!isStopped) {
            std::unique_lock<std::mutex> guard(m_locker);
            isStopped = true;
            guard.unlock();
            cond.notify_all();
            if (m_thr.joinable()) {
                m_thr.join();
            }
        }
    }

    void Timer::run_once() {
        isStopped = false;
        m_thr = std::thread([&]() {
            std::unique_lock<std::mutex> guard(m_locker);
            if (cond.wait_for(guard, m_interval, [&](){return isStopped;})) {
                return;
            }
            guard.unlock();
            if (m_handler) {
                m_handler();
            }
        });
    }

    // time_duration
    time_duration::time_duration(bool _start):
        m_run(false)
    {
        if (_start)
            start();
    }

    time_duration::time_duration(bool _start, std::string message):
        m_run(false),
        m_msg(message)
    {
        if (_start)
            start();
    }

    time_duration::~time_duration()
    {
        stop();
    }

    void time_duration::start()
    {
        if (!m_run)
        {
            m_run = true;
            m_start = boost::posix_time::microsec_clock::local_time();
        }
    }

    void time_duration::stop()
    {
        if (m_run)
        {
            m_run = false;
            LOGDEBUG << m_msg << " " << (boost::posix_time::microsec_clock::local_time() - m_start).total_milliseconds() << " millisec";
        }
    }
}
