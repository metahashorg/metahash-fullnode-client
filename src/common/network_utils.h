#ifndef NETWORK_UTILS_H_
#define NETWORK_UTILS_H_

#include <string>

namespace common {

std::string getMyIp();

std::string getHostName();

std::string getMyIp2(const std::string &externalServer);

}

#endif // NETWORK_UTILS_H_
