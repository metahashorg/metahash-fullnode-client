#pragma once

#include "boost/format.hpp"
#include <string.h>
//#include <string>

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

    void parse_address(const std::string& address, std::string& host, std::string& port);
    bool gen_sign(std::string& result, const std::string& prv_key, const char* fmt, ...);
}
