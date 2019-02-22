#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <string>
#include <cstring>

namespace string_utils
{

size_t get_size(const char*  val) noexcept;

template <typename T>
size_t get_size(const T& val) noexcept {
    return val.size();
}

template <typename T1, typename T2>
std::string str_concat(const T1& v1, const T2& v2) noexcept {
    std::string res;
    res.reserve(get_size(v1) + get_size(v2));
    res.append(v1);
    res.append(v2);
    return res;
}

template <typename T1, typename T2, typename T3>
std::string str_concat(const T1& v1, const T2& v2, const T3& v3) noexcept {
    std::string res;
    res.reserve(get_size(v1) + get_size(v2) + get_size(v3));
    res.append(v1);
    res.append(v2);
    res.append(v3);
    return res;
}

template <typename T1, typename T2, typename T3, typename T4>
std::string str_concat(const T1& v1, const T2& v2, const T3& v3, const T4& v4) noexcept {
    std::string res;
    res.reserve(get_size(v1) + get_size(v2) + get_size(v3) + get_size(v4));
    res.append(v1);
    res.append(v2);
    res.append(v3);
    res.append(v4);
    return res;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::string str_concat(const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) noexcept {
    std::string res;
    res.reserve(get_size(v1) + get_size(v2) + get_size(v3) + get_size(v4) + get_size(v5));
    res.append(v1);
    res.append(v2);
    res.append(v3);
    res.append(v4);
    res.append(v5);
    return res;
}

template <typename T1, typename T2>
void str_append(std::string& res, const T1& v1, const T2& v2) noexcept {
    size_t sz = get_size(v1) + get_size(v2);
    if (res.capacity() < sz) {
        res.reserve(sz);
    }
    res.append(v1);
    res.append(v2);
}

template <typename T1, typename T2, typename T3>
void str_append(std::string& res, const T1& v1, const T2& v2, const T3& v3) noexcept {
    size_t sz = get_size(v1) + get_size(v2) + get_size(v3);
    if (res.capacity() < sz) {
        res.reserve(sz);
    }

    res.append(v1);
    res.append(v2);
    res.append(v3);
}

template <typename T1, typename T2, typename T3, typename T4>
void str_append(std::string& res, const T1& v1, const T2& v2, const T3& v3, const T4& v4) noexcept {
    size_t sz = get_size(v1) + get_size(v2) + get_size(v3) + get_size(v4);
    if (res.capacity() < sz) {
        res.reserve(sz);
    }
    res.append(v1);
    res.append(v2);
    res.append(v3);
    res.append(v4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void str_append(std::string& res, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5) noexcept {
    size_t sz = get_size(v1) + get_size(v2) + get_size(v3) + get_size(v4) + get_size(v5);
    if (res.capacity() < sz) {
        res.reserve(sz);
    }
    res.append(v1);
    res.append(v2);
    res.append(v3);
    res.append(v4);
    res.append(v5);
}

void bin2hex(const unsigned char* buf, size_t size, char* res) noexcept;

//void get_size(size_t& size) {
//}

//template<typename... args>
//void get_size(size_t& size, const char* v, args... Fargs) {
//    size += strlen(v);
//    get_size(size, Fargs...);
//}

//void get_size(size_t& size, std::string_view& v, const char*& c) {
//    size += v.size();
//    get_size(size, c);
//}

//template<typename T, typename... args>
//void get_size(size_t& size, const T& v, args... Fargs) {
//    size += v.size();
//    get_size(size, Fargs...);
//}

//void string_append(std::string& str) {
//}

//template<typename... args>
//void string_append(std::string& str, const char* v, args... Fargs) {
//    str.append(v);
//    string_append(str, Fargs...);
//}

//void string_append(std::string& str, std::string_view& v, const char*& c) {
//    str.append(v.data(), v.size());
//    string_append(str, c);
//}

//template<typename T, typename... args>
//void string_append(std::string& str, const T& v, args... Fargs) {
//    str.append(v.data(), v.size());
//    string_append(str, Fargs...);
//}

//template<typename T, typename... args>
//std::string concat(const T& value, args... Fargs)
//{
//    std::string result;
//    size_t size(0);
//    get_size(size, Fargs...);
//    result.reserve(size);
//    string_append(result, Fargs...);
//    return result;
//}

}

#endif // STRING_UTILS_H_
