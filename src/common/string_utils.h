#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <string>
#include <cstring>
#include <vector>

#include <iostream>

namespace string_utils
{

size_t get_size(const char* val) noexcept;
size_t get_size(char* val) noexcept;

template <typename T>
size_t get_size(const T& val) noexcept {
    return val.size();
}

void check_size(size_t& size);

template<typename T, typename... args>
void check_size(size_t& size, const T& v, args... Targs) {
    size_t sz = get_size(v);
    // overflow
    if (std::numeric_limits<size_t>::max() - size < sz) {
        size = 0;
        return;
    }
    size += sz;
    check_size(size, Targs...);
}

void str_append_unsafe(std::string& str);

template<typename T, typename... args>
void str_append_unsafe(std::string& str, const T& v, args... Targs) {
    str.append(v);
    str_append_unsafe(str, Targs...);
}

template <typename... args>
void str_append(std::string& res, args... Targs) noexcept {
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
std::string str_concat(args... Targs) noexcept {
    std::string res;
    str_append(res, Targs...);
    return res;
}

//template <typename T1, typename T2>
//std::string str_concat(const T1& v1, const T2& v2) noexcept {
//    std::string res;
//    size_t sz = 0;
//    check_size(sz, v1, v2);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//    }
//    return res;
//}

//template <typename T1, typename T2, typename T3>
//std::string str_concat(const T1& v1, const T2& v2, const T3& v3) noexcept {
//    std::string res;
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//    }
//    return res;
//}

//template <typename T1, typename T2, typename T3, typename T4>
//std::string str_concat(const T1& v1, const T2& v2, const T3& v3, const T4& v4) noexcept {
//    std::string res;
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3, v4);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//        res.append(v4);
//    }
//    return res;
//}

//template <typename T1, typename T2, typename T3, typename T4, typename T5>
//std::string str_concat(const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) noexcept {
//    std::string res;
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3, v4, v5);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//        res.append(v4);
//        res.append(v5);
//    }
//    return res;
//}

//template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
//std::string str_concat(const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6) noexcept {
//    std::string res;
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3, v4, v5, v6);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//        res.append(v4);
//        res.append(v5);
//        res.append(v6);
//    }
//    return res;
//}

//template <typename T1, typename T2>
//void str_append(std::string& res, const T1& v1, const T2& v2) noexcept {
//    size_t sz = 0;
//    check_size(sz, v1, v2);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//    }
//}

//template <typename T1, typename T2, typename T3>
//void str_append(std::string& res, const T1& v1, const T2& v2, const T3& v3) noexcept {
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//    }
//}

//template <typename T1, typename T2, typename T3, typename T4>
//void str_append(std::string& res, const T1& v1, const T2& v2, const T3& v3, const T4& v4) noexcept {
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3, v4);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//        res.append(v4);
//    }
//}

//template <typename T1, typename T2, typename T3, typename T4, typename T5>
//void str_append(std::string& res, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) noexcept {
//    size_t sz = 0;
//    check_size(sz, v1, v2, v3, v4, v5);
//    if (sz != 0) {
//        if (res.capacity() < sz) {
//            res.reserve(sz);
//        }
//        res.append(v1);
//        res.append(v2);
//        res.append(v3);
//        res.append(v4);
//        res.append(v5);
//    }
//}

std::string bin2hex(const std::vector<unsigned char>& src);
void bin2hex(const unsigned char* buf, size_t size, char* res);
void bin2hex(const std::string& buf, std::string& res);

void hex2val_le(std::string_view& buf, size_t size);

std::string to_lower(const std::string &str);
std::string to_upper(const std::string &str);

std::string to_lower(const char* buf, size_t size);

}

#endif // STRING_UTILS_H_
