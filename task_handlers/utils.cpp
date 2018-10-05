#include <stdarg.h>
#include "utils.h"
#include "../cpplib_open_ssl_decor/crypto.h"
#include <iostream>

namespace utils
{
    void parse_address(const std::string& address, std::string& host, std::string& port)
    {
        auto pos = std::find(address.begin(), address.end(), ':');
        if (pos != address.end())
        {
            port.clear();
            port.append(pos + 1, address.end());
        }
        host.clear();
        host.append(address.begin(), pos);
    }

    bool gen_sign(std::string& result, const std::string& prv_key, const char* fmt, ...)
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
            return true;
        }
        catch (std::exception& e)
        {
            result = "generate sign failed: ";
            result.append(e.what());
            return false;
        }
    }

    void Timer::start(const Interval& interval, const Handler& handler)
    {
        m_handler = handler;
        m_thr = std::thread([&, interval]()
        {
            auto fut = m_promise.get_future();
            if (fut.wait_for(interval) == std::future_status::timeout)
                if (m_handler)
                    m_handler();
        });
        m_thr.detach();
    }

    void Timer::stop()
    {
        m_promise.set_value();
    }
}
