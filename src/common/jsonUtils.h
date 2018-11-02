#ifndef JSON_UTILS_H_
#define JSON_UTILS_H_

#include <string>

#include <rapidjson/document.h>

std::string jsonToString(const rapidjson::Value &doc, bool isFormat=false);

rapidjson::Value strToJson(const std::string &val, rapidjson::Document::AllocatorType& allocator);

#endif // JSON_UTILS_H_
