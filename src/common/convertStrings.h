#ifndef CONVERT_STRINGS_H_ 
#define CONVERT_STRINGS_H_

#include <string>
#include <vector>

#include "BigInteger.h"

template<typename Iterator>
inline std::string toHex(Iterator begin, Iterator end) {
    std::string result;
    result.reserve(std::distance(begin, end) * 2);
    for (Iterator curr = begin; curr != end; curr++) {
        const unsigned char c = *curr;
        const unsigned char mod = c % 16;
        const unsigned char div = c / 16;
        result += div <= 9 ? div + '0' : div - 10 + 'a';
        result += mod <= 9 ? mod + '0' : mod - 10 + 'a';
    }
    return result;
}

inline std::string toHex(const std::vector<unsigned char> &vec) {
    std::string result;
    result.reserve(vec.size() * 2);
    for (const unsigned char c: vec) {
        const unsigned char mod = c % 16;
        const unsigned char div = c / 16;
        result += div <= 9 ? div + '0' : div - 10 + 'a';
        result += mod <= 9 ? mod + '0' : mod - 10 + 'a';
    }
    return result;
}

inline std::vector<unsigned char> fromHex(const std::string &hex) {
    size_t begin = 0;
    if (hex.compare(0, 2, "0x") == 0) {
        begin = 2;
    }
    std::vector<unsigned char> result;
    if (hex.size() % 2 != 0) {
        return result;
    }
    result.reserve(hex.size() / 2);
    for (size_t i = begin; i < hex.size(); i += 2) {
        unsigned char mod = hex[i + 1];
        mod = ('0' <= mod && mod <= '9' ? mod - '0' : ('a' <= mod && mod <= 'f' ? mod - 'a' + 10 : mod - 'A' + 10));
        unsigned char div = hex[i];
        div = ('0' <= div && div <= '9' ? div - '0' : ('a' <= div && div <= 'f' ? div - 'a' + 10 : div - 'A' + 10));
        result.push_back(div * 16 + mod);
    }
    return result;
}

inline std::string uintToHexStr(size_t num) {
    std::string res;
    if (num == 0) {
        res = "0x0";
        return res;
    }
    while (num != 0) {
        const uint8_t mod = num % 16;
        const char cc = (mod <= 9 ? '0' + mod : 'a' + mod - 10);
        res = cc + res;
        num /= 16;
    }
    res = "0x" + res;
    return res;
}

inline std::string uintToHexStr(BigInteger num) {
    std::string res;
    if (num == 0) {
        res = "0x0";
        return res;
    }
    while (num != 0) {
        const BigInteger modBiInt = num % 16;
        const uint8_t mod = modBiInt.get_si();
        const char cc = (mod <= 9 ? '0' + mod : 'a' + mod - 10);
        res = cc + res;
        num /= 16;
    }
    res = "0x" + res;
    return res;
}

template<typename Type>
inline Type hexStrToInt(const std::string &str) {
    std::string copy = str;
    if (copy.find("0x") == 0) {
        copy = copy.substr(2);
    }
    
    Type result = 0;
    for (char c: copy) {
        result *= 16;
        result += ('0' <= c && c <= '9' ? (c - '0') :
        ('a' <= c && c <= 'f' ? (c - 'a' + 10) :
        ('A' <= c && c <= 'F' ? (c - 'A' + 10) : 0)));
    }
    
    return result;
}

#endif // CONVERT_STRINGS_H_
