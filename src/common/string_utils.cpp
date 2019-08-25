#include "string_utils.h"
#include <algorithm>

namespace string_utils
{
size_t get_size(const char* val) {
    return strlen(val);
}

size_t get_size(char* val) {
    return strlen(val);
}

void check_size(size_t&) noexcept {
}

void str_append_unsafe(std::string&) noexcept {
}

namespace {
    static const char hex[] = {
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
    };
}

std::string bin2hex(const std::vector<unsigned char>& src)
{
    std::string res;
    res.reserve(src.size() * 2 + 1);
    for (size_t i = 0, size = src.size(); i < size; i++) {
        const char* c = hex + static_cast<unsigned char>(src[i])*2;
        res.insert(res.end(), c, c + 2);
    }
    return res;
}

void bin2hex(const unsigned char* buf, size_t size, char* res)
{
    for (size_t i = 0; i < size; i++) {
        const char* c = hex + (*(buf+i))*2;
        res[i*2] = *c;
        res[i*2+1] = *(c+1);
    }
}

void bin2hex(const std::string& buf, std::string& res)
{
    res.reserve(buf.size() * 2);
    for (size_t i = 0, size = buf.size(); i < size; i++) {
        const char* c = hex + static_cast<unsigned char>(buf[i])*2;
        res.push_back(*c);
        res.push_back(*(c+1));
    }
}

void hex2val_le(std::string_view& buf, size_t size)
{
    uint64_t res = 0;
    unsigned char c;
    for (size_t i = size/2; i > 0; --i) {
        c = static_cast<unsigned char>(buf[i]);
        res *= 0x10;
        res += ('0' <= c && c <= '9' ? (c - '0') :
        ('a' <= c && c <= 'f' ? (c - 'a' + 10) :
        ('A' <= c && c <= 'F' ? (c - 'A' + 10) : 0)));
    }
}

std::string to_lower(const std::string &str)
{
    std::string res;
    res.reserve(str.size() + 1);
    std::transform(str.begin(), str.end(), std::back_inserter(res), tolower);
    return res;
}

std::string to_lower(const char* buf, size_t size)
{
    std::string res;
    res.reserve(size + 1);
    for (size_t i = 0; i < size; i++) {
        res.push_back(static_cast<char>(tolower(static_cast<int>(*(buf+i)))));
    }
    return res;
}

std::string to_upper(const std::string &str)
{
    std::string res;
    res.reserve(str.size() + 1);
    std::transform(str.begin(), str.end(), std::back_inserter(res), toupper);
    return res;
}

}
