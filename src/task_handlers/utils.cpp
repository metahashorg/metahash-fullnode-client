#include <stdarg.h>
#include "utils.h"
#include "../cpplib_open_ssl_decor/crypto.h"
#include <iostream>
#include "time_duration.h"
#include "openssl/sha.h"
#include "common/convertStrings.h"

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

    uint64_t read_compact_int(std::string_view& buf) {
        if (buf.size() < 2) {
            return 0;
        }

        std::string tmp;
        if (buf.compare(0, 2, "fa") == 0) {
            // uint16_t
            if (buf.size() < sizeof(uint16_t)*2 + 2) {
                throw std::invalid_argument("Can not read compact uint16");
            }
            buf.remove_prefix(2);
            tmp.assign(buf.data(), sizeof(uint16_t)*2);
            buf.remove_prefix(sizeof(uint16_t)*2);
        } else if (buf.compare(0, 2, "fb") == 0) {
            // uint32_t
            if (buf.size() < sizeof(uint32_t)*2 + 2) {
                throw std::invalid_argument("Can not read compact uint32");
            }
            buf.remove_prefix(2);
            tmp.assign(buf.data(), sizeof(uint32_t)*2);
            buf.remove_prefix(sizeof(uint32_t)*2);
        } else if (buf.compare(0, 2, "fc") == 0) {
            // uint64_t
            if (buf.size() < sizeof(uint64_t)*2 + 2) {
                throw std::invalid_argument("Can not read compact uint64");
            }
            buf.remove_prefix(2);
            tmp.assign(buf.data(), sizeof(uint64_t)*2);
            buf.remove_prefix(sizeof(uint64_t)*2);
        } else {
            // uint8_t
            if (buf.size() < sizeof(uint8_t)*2 + 2) {
                throw std::invalid_argument("Can not read compact uint8");
            }
            tmp.assign(buf.data(), sizeof(uint8_t)*2);
            buf.remove_prefix(sizeof(uint8_t)*2);
        }
        reverse_byte_order(tmp);
        return common::hexStrToInt<uint64_t>(tmp);
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
                case 'u':
                    utils::write_compact_int(va_arg(args, uint32_t), data);
                    break;
                case 'U':
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

    bool make_tx(std::string& result, const char* fmt, ...)
    {
        try {
            std::vector<unsigned char> data;
            va_list args;
            va_start(args, fmt);
            while (*fmt != '\0') {
                switch(*fmt) {
                case 'd':
                    utils::write_compact_int(va_arg(args, uint32_t), data);
                    break;
                case 'D':
                    utils::write_compact_int(va_arg(args, uint64_t), data);
                    break;
                case 'u':
                    utils::write_compact_int(va_arg(args, uint32_t), data);
                    break;
                case 'U':
                    utils::write_compact_int(va_arg(args, uint64_t), data);
                    break;
                case 'x': {
                    std::string param = va_arg(args, char*);
                    if (param.size() > 2) {
                        if (param[0] == '0' && param[1] == 'x')
                            param.erase(0, 2);
                    }
                    auto bin = hex2bin(param);
                    data.insert(std::end(data), std::begin(bin), std::end(bin));
                    break;
                }
                case 's': {
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

            std::vector<unsigned char> hash;
            hash.resize(SHA256_DIGEST_LENGTH);
            SHA256_CTX ctx;
            SHA256_Init(&ctx);
            SHA256_Update(&ctx, data.data(), data.size());
            SHA256_Final(hash.data(), &ctx);
            data = hash;
            SHA256_Init(&ctx);
            SHA256_Update(&ctx, data.data(), data.size());
            SHA256_Final(hash.data(), &ctx);

            result = bin2hex(hash);
            return true;
        } catch (std::exception& e) {
            LOGERR << "make tx failed: " << e.what();
            return false;
        } catch (...) {
            LOGERR << "make tx failed";
            return false;
        }
    }

    bool parse_tansaction(std::string_view transaction, std::string& to, uint64_t& value, uint64_t& fee, uint64_t& nonce, uint64_t& data_size, std::string& data)
    {
        try {
            to.reserve(52);
            to.append("0x");
            to.append(transaction.data(), 50);
            transaction.remove_prefix(50);
            value = read_compact_int(transaction);
            fee = read_compact_int(transaction);
            nonce = read_compact_int(transaction);
            data_size = read_compact_int(transaction);
            data.assign(transaction.data(), transaction.size());
            return true;
        } catch (std::exception& e) {
            LOGERR << "parse transaction failed: " << e.what();
            return false;
        }
    }

    void reverse_byte_order(std::string& hex)
    {
        char c;
        for (size_t i = 0, sz = hex.size(), cnt = sz/2 - (sz/2 % 2); i < cnt; ++i) {
            c = hex[i];
            if (i % 2) {
                hex[i] = hex[sz-i];
                hex[sz-i] = c;
            } else {
                hex[i] = hex[sz-i-2];
                hex[sz-i-2] = c;
            }
        }
    }

    // Timer
    Timer::~Timer() {
        if (m_thr.joinable()) {
            m_thr.join();
        }
    }

    void Timer::start(const Interval& interval, const Handler& handler, bool immediately /*= true*/) {
        std::unique_lock<std::mutex> guard(m_locker);
        set_callback(handler);
        m_interval = interval;
        if (immediately) {
            isStopped = false;
            run_once();
        } else {
            isStopped = true;
        }
    }

    void Timer::stop() {
        std::unique_lock<std::mutex> guard(m_locker);
        if (!isStopped) {
            isStopped = true;
            cond.notify_one();
            guard.unlock();
            if (m_thr.joinable()) {
                m_thr.join();
            }
        }
        set_callback(nullptr);
    }

    void Timer::run_once() {
        isStopped = false;
        if (m_thr.joinable()) {
            m_thr.join();
        }
        m_thr = std::thread([&]() {
            std::unique_lock<std::mutex> guard(m_locker);
            if (cond.wait_for(guard, m_interval, [&](){return isStopped;})) {
                return;
            }
            isStopped = true;
//            guard.unlock();
            if (m_handler) {
                m_handler();
            }
        });
    }

    void Timer::set_callback(const Handler& handler) {
        m_handler = handler;
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
            gettimeofday(&m_start, NULL);
        }
    }

    void time_duration::stop()
    {
        if (m_run)
        {
            m_run = false;
            struct timeval end;
            gettimeofday(&end, NULL);
            long elapsed = ((end.tv_sec - m_start.tv_sec) * 1000) + (end.tv_usec / 1000 - m_start.tv_usec / 1000);
            LOGDEBUG << m_msg << " " << elapsed << " millisec";
        }
    }
}
