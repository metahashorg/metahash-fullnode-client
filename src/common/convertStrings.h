#ifndef CONVERT_STRINGS_H_ 
#define CONVERT_STRINGS_H_

#include <string>
#include <vector>

#include "BigInteger.h"

namespace common {

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

const char HexLookup2[513] = {
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
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"};
    
const unsigned char DecLookup2[256] = {
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // gap before first hex digit
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // 0123456789
    0,  0,  0,  0,  0,  0,  0, // :;<=>?@ (gap)
    10, 11, 12, 13, 14, 15, // ABCDEF
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, // GHIJKLMNOPQRS (gap)
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, // TUVWXYZ[/]^_` (gap)
    10, 11, 12, 13, 14, 15, // abcdef
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // fill zeroes
};
    
inline void bin2hex2(const unsigned char* src, size_t len, char* dst) noexcept {
    for (size_t i = 0; i < len; i++) {
        const char* hex = HexLookup2 + 2 * src[i];
        dst[i * 2] = *hex;
        dst[i * 2 + 1] = *(hex + 1);
    }
}

inline std::string toHex(const std::vector<unsigned char> &vec) {
    std::string result;
    result.resize(vec.size() * 2);
    bin2hex2(vec.data(), vec.size(), result.data());
    return result;
}

inline size_t hex2bin2(const char* src, size_t len, unsigned char* dst) noexcept {
    auto* data = reinterpret_cast<const unsigned char*>(src);
    
    size_t dst_len = 0;
    for (size_t i = 0; i < len; i += 2) {
        unsigned char d = DecLookup2[data[i]] << 4u;
        d |= DecLookup2[data[i + 1]];
        
        dst[dst_len] = d;
        dst_len++;
    }
    
    return dst_len;
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
    result.resize((hex.size() - begin) / 2);
    hex2bin2(hex.data() + begin, hex.size() - begin, result.data());
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

}

#endif // CONVERT_STRINGS_H_
