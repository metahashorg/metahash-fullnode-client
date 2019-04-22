#ifndef NS_LOOKUP_H_
#define NS_LOOKUP_H_

#include <string>

std::string getBestIp(const std::string &address, const char* print = nullptr);

void lookup_best_ip();

#endif // NS_LOOKUP_H_
