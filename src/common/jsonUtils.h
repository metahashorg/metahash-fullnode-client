#ifndef JSON_UTILS_H_
#define JSON_UTILS_H_

#include <string>
#include <optional>
#include <string_view>

#include <rapidjson/document.h>

#include "check.h"

namespace common {
    
std::string jsonToString(const rapidjson::Value &doc, bool isFormat=false);

rapidjson::Value strToJson(const std::string &val, rapidjson::Document::AllocatorType& allocator);

struct JsonArray {};

struct JsonObject {};

template<class T> struct dependent_false : std::false_type {};

template<typename T>
std::optional<std::conditional_t<std::is_same_v<T, JsonArray>, rapidjson::Value::ConstArray, std::conditional_t<std::is_same_v<T, JsonObject>, rapidjson::Value::ConstObject, T>>> getOpt(const rapidjson::Value &obj) {
    const auto &o = obj;
    if constexpr (std::is_same_v<T, std::string>) {
        if (o.IsString()) {
            return o.GetString();
        }
    } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int>) {
        if (o.IsInt()) {
            return o.GetInt();
        }
    } else if constexpr (std::is_same_v<T, long int> || std::is_same_v<T, unsigned long int> || std::is_same_v<T, long long int> || std::is_same_v<T, unsigned long long int>) {
        if (o.IsInt64()) {
            return o.GetInt64();
        }
    } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
        if (o.IsDouble()) {
            return o.GetDouble();
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        if (o.IsBool()) {
            return o.GetBool();
        }
    } else if constexpr (std::is_same_v<T, JsonArray>) {
        if (o.IsArray()) {
            return o.GetArray();
        }
    } else if constexpr (std::is_same_v<T, JsonObject>) {
        if (o.IsObject()) {
            return o.GetObject();
        }
    } else {
        static_assert(dependent_false<T>::value, "Incorrect type");
    }
    return std::nullopt;
}

template<typename T>
auto getOpt(const rapidjson::Value::ConstObject &obj, std::string_view name) -> decltype(getOpt<T>(obj[name.data()])) {
    if (!obj.HasMember(name.data())) {
        return std::nullopt;
    }
    return getOpt<T>(obj[name.data()]);
}

template<typename T, typename Exception=UserException>
auto getOpt(const rapidjson::Document &obj, std::string_view name) -> decltype(getOpt<T>(obj, name)) {
    CHECK1(obj.IsObject(), "Json document not object", Exception);
    return getOpt<T>(obj.GetObject(), name);
}

template<typename T>
T getOpt(const rapidjson::Value::ConstObject &obj, std::string_view name, T defaultValue) {
    const auto result = getOpt<T>(obj, name);
    if (result.has_value()) {
        return result.value();
    } else {
        return defaultValue;
    }
}

template<typename T, typename Exception=UserException>
auto get(const rapidjson::Value::ConstObject &obj, std::string_view name) -> typename std::remove_reference<decltype(getOpt<T>(obj, name).value())>::type {
    const auto result = getOpt<T>(obj, name);
    CHECK1(result.has_value(), "Field " + std::string(name) + " not found", Exception);
    return result.value();
}

template<typename T, typename Exception=UserException>
auto get(const rapidjson::Document &obj, std::string_view name) -> decltype(get<T, Exception>(obj.GetObject(), name)) {
    CHECK1(obj.IsObject(), "Json document not object", Exception);
    return get<T, Exception>(obj.GetObject(), name);
}

template<typename T, typename Exception=UserException>
auto get(const rapidjson::Value &obj) -> typename std::remove_reference<decltype(getOpt<T>(obj).value())>::type {
    const auto result = getOpt<T>(obj);
    CHECK1(result.has_value(), "Incorrect json", Exception);
    return result.value();
}

}

#endif // JSON_UTILS_H_
