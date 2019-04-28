#ifndef NS_LOOKUP_H_
#define NS_LOOKUP_H_

#include <string>

struct NsResult {
    std::string server;
    unsigned long long timeout;

    NsResult()
        : timeout(0)
    {}
    NsResult(const std::string &server, unsigned long long timeout)
        : server(server)
        , timeout(timeout)
    {}
};

NsResult getBestIp(const std::string &address, const char* print = nullptr);

void lookup_best_ip();

#endif // NS_LOOKUP_H_
