#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <string>
#include <cstring>
#include <vector>

#include <iostream>

namespace string_utils
{

size_t get_size(const char* val);
size_t get_size(char* val);

template <typename T>
size_t get_size(const T& val) {
    return val.size();
}

template<typename T, typename... args>
void check_size(size_t& size, const T& v, args... Targs) {
    size_t sz = get_size(v);
    // overflow
    if (std::numeric_limits<size_t>::max() - size < sz) {
        size = 0;
        return;
    }
    size += sz;
    if constexpr(sizeof...(Targs) > 0) {
        check_size(size, Targs...);
    }
}

template<typename T, typename... args>
void str_append_unsafe(std::string& str, const T& v, args... Targs) {
    str.append(v);
    if constexpr(sizeof...(Targs) > 0) {
        str_append_unsafe(str, Targs...);
    }
}

template <typename... args>
void str_append(std::string& res, args... Targs) {
    size_t sz = 0;
    check_size(sz, Targs...);
    if (sz != 0) {
        if (res.capacity() < sz) {
            res.reserve(sz);
        }
        str_append_unsafe(res, Targs...);
    }
}

template <typename... args>
std::string str_concat(args... Targs) {
    std::string res;
    str_append(res, Targs...);
    return res;
}

std::string bin2hex(const std::vector<unsigned char>& src);
void bin2hex(const unsigned char* buf, size_t size, char* res);
void bin2hex(const std::string& buf, std::string& res);

void hex2val_le(std::string_view& buf, size_t size);

std::string to_lower(const std::string &str);
std::string to_upper(const std::string &str);

std::string to_lower(const char* buf, size_t size);

}

#endif // STRING_UTILS_H_
