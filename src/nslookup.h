#ifndef NS_LOOKUP_H_
#define NS_LOOKUP_H_

#include <string>
#include <vector>

struct NsResult {
    std::string server;
    unsigned long long timeout;
    unsigned int blocks_count;

    NsResult()
        : timeout(0)
    {}
    NsResult(const std::string &server, unsigned long long timeout)
        : server(server)
        , timeout(timeout)
        , blocks_count(0)
    {}
};

bool get_ip_addresses(const std::string &address, std::vector<NsResult>& ip);
void lookup_best_ip();

#endif // NS_LOOKUP_H_
