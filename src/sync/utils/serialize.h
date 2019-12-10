#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <variant>

#include "check.h"

namespace torrent_node_lib {

// Процедуры сериализации сделаны таким образом, чтобы младшие разряды оказывались в конце
template<typename T>
[[nodiscard]] inline std::string toBinaryStringBigEndian(T val) {
    static_assert(std::is_unsigned_v<T>, "Not unsigned integer");
    std::string result(sizeof(T), 0);
    int i = result.size() - 1;
    while (val != 0 && i >= 0) {
        result[i] = val % 256;
        val /= 256;
        i--;
    }
    return result;
}

// Процедуры сериализации сделаны таким образом, чтобы младшие разряды оказывались в конце
template<typename T>
inline void toBinaryStringBigEndian(T val, std::vector<char> &buffer) {
    static_assert(std::is_unsigned_v<T>, "Not unsigned integer");
    int i = sizeof(val) - 1;
    const size_t oldSize = buffer.size();
    buffer.resize(oldSize + sizeof(val), 0);
    while (val != 0 && i >= 0) {
        buffer[oldSize + i] = val % 256;
        val /= 256;
        i--;
    }
}

template<typename T>
inline T fromBinaryStringBigEndian(const std::string &raw, size_t fromPos, size_t &endPos) {
    static_assert(std::is_unsigned_v<T>, "Not unsigned integer");
    endPos = fromPos;
    constexpr size_t sizeField = sizeof(T);
    if (raw.size() < sizeField + fromPos) {
        return 0;
    }
    
    T val = 0;
    for (size_t i = 0; i < sizeField; i++) {
        val *= 256;
        val += (unsigned char)raw[i + fromPos];
    }
    endPos = fromPos + sizeField;
    return val;
}

template<typename T>
[[nodiscard]] inline std::string serializeIntBigEndian(T intValue) {
    return toBinaryStringBigEndian<T>(intValue);
}

template<typename T>
inline void serializeIntBigEndian(T intValue, std::vector<char> &buffer) {
    toBinaryStringBigEndian<T>(intValue, buffer);
}

[[nodiscard]] inline std::string serializeStringBigEndian(const std::string &str) {
    std::string res;
    res.reserve(str.size() + 10);
    res += serializeIntBigEndian<size_t>(str.size());
    res += str;
    return res;
}

[[nodiscard]] inline std::string serializeVectorBigEndian(const std::vector<unsigned char> &str) {
    std::string res;
    res.reserve(str.size() + 10);
    res += serializeIntBigEndian<size_t>(str.size());
    res.insert(res.end(), str.begin(), str.end());
    return res;
}

inline void serializeVectorBigEndian(const std::vector<unsigned char> &str, std::vector<char> &buffer) {
    serializeIntBigEndian<size_t>(str.size(), buffer);
    buffer.insert(buffer.end(), str.begin(), str.end());
}

inline void serializeStringBigEndian(const std::string &str, std::vector<char> &buffer) {
    serializeIntBigEndian<size_t>(str.size(), buffer);
    buffer.insert(buffer.end(), str.begin(), str.end());
}

template<typename T>
inline T deserializeIntBigEndian(const std::string &raw, size_t fromPos, size_t &endPos) {
    const T val = fromBinaryStringBigEndian<T>(raw, fromPos, endPos);
    return val;
}

template<typename T>
inline T deserializeIntBigEndian(const std::string &raw, size_t &fromPos) {
    size_t endPos = fromPos;
    const T val = deserializeIntBigEndian<T>(raw, fromPos, endPos);
    CHECK(endPos != fromPos, "Incorrect raw");
    fromPos = endPos;
    return val;
}

inline std::string deserializeStringBigEndian(const std::string &raw, size_t fromPos, size_t &endPos) {
    const size_t sizeString = deserializeIntBigEndian<size_t>(raw, fromPos, endPos);
    if (fromPos == endPos) {
        return "";
    }
    fromPos = endPos;
    
    const std::string str = raw.substr(fromPos, sizeString);
    endPos += sizeString;
    
    return str;
}

inline std::vector<unsigned char> deserializeVectorBigEndian(const std::string &raw, size_t fromPos, size_t &endPos) {
    const size_t sizeString = deserializeIntBigEndian<size_t>(raw, fromPos, endPos);
    if (fromPos == endPos) {
        return std::vector<unsigned char>();
    }
    fromPos = endPos;
    
    const std::string tmp = raw.substr(fromPos, sizeString);
    const std::vector<unsigned char> str(tmp.begin(), tmp.end());
    endPos += sizeString;
    
    return str;
}

inline std::string deserializeStringBigEndian(const std::string &raw, size_t &fromPos) {
    size_t endPos = fromPos;
    const std::string str = deserializeStringBigEndian(raw, fromPos, endPos);
    CHECK(endPos != fromPos, "Incorrect raw");
    fromPos = endPos;
    
    return str;
}

inline std::vector<unsigned char> deserializeVectorBigEndian(const std::string &raw, size_t &fromPos) {
    size_t endPos = fromPos;
    const std::vector<unsigned char> str = deserializeVectorBigEndian(raw, fromPos, endPos);
    CHECK(endPos != fromPos, "Incorrect raw");
    fromPos = endPos;
    
    return str;
}


template<typename T>
[[nodiscard]] inline std::string serializeInt(T intValue) {
    return serializeIntBigEndian<T>(intValue);
}

template<typename T>
inline void serializeInt(T intValue, std::vector<char> &buffer) {
    serializeIntBigEndian<T>(intValue, buffer);
}

[[nodiscard]] inline std::string serializeString(const std::string &str) {
    return serializeStringBigEndian(str);
}

[[nodiscard]] inline std::string serializeVector(const std::vector<unsigned char> &str) {
    return serializeVectorBigEndian(str);
}

inline void serializeVector(const std::vector<unsigned char> &str, std::vector<char> &buffer) {
    serializeVectorBigEndian(str, buffer);
}

inline void serializeString(const std::string &str, std::vector<char> &buffer) {
    serializeStringBigEndian(str, buffer);
}

template<typename T>
inline T deserializeInt(const std::string &raw, size_t fromPos, size_t &endPos) {
    return deserializeIntBigEndian<T>(raw, fromPos, endPos);
}

template<typename T>
inline T deserializeInt(const std::string &raw, size_t &fromPos) {
    return deserializeIntBigEndian<T>(raw, fromPos);
}

inline std::string deserializeString(const std::string &raw, size_t fromPos, size_t &endPos) {
    return deserializeStringBigEndian(raw, fromPos, endPos);
}

inline std::string deserializeString(const std::string &raw, size_t &fromPos) {    
    return deserializeStringBigEndian(raw, fromPos);
}

inline std::vector<unsigned char> deserializeVector(const std::string &raw, size_t &fromPos) {    
    return deserializeVectorBigEndian(raw, fromPos);
}

template<typename Variant>
inline void serializeVariant(const Variant &variant, std::vector<char> &buffer) {
    serializeInt<uint64_t>(variant.index(), buffer);
    std::visit([&buffer](auto &&arg){
        if constexpr (!std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>) {
            arg.serialize(buffer);
        }
    }, variant);
}

template<size_t I, typename Variant>
void tryParse(const std::string &raw, size_t &fromPos, size_t number, Variant &status) {
    if (I == number) {
        using TypeElement = std::decay_t<decltype(std::get<I>(status))>;
        if constexpr (!std::is_same_v<TypeElement, std::monostate>) {
            status = TypeElement::deserialize(raw, fromPos);
        }
    }
}

template <typename Variant, std::size_t ... I>
void parseVarintImpl(const std::string &raw, size_t &fromPos, size_t number, Variant &status, std::index_sequence<I ... >) {
    (tryParse<I>(raw, fromPos, number, status), ...);
}

template <typename Variant, std::size_t ... I>
void deserializeVarint(const std::string &raw, size_t &fromPos, Variant &status) {
    constexpr size_t varintSize = std::variant_size_v<std::decay_t<decltype(status)>>;
    const size_t number = deserializeInt<size_t>(raw, fromPos);
    CHECK(number < varintSize, "Incorrect type in transaction status");
    
    parseVarintImpl(raw, fromPos, number, status, std::make_index_sequence<varintSize>());
}

}

#endif // SERIALIZE_H_
