#ifndef MODULES_H_
#define MODULES_H_

#include <bitset>
#include <string>
#include <set>

namespace torrent_node_lib {
    
using Modules = std::bitset<7>;

extern const int MODULE_BLOCK;
extern const int MODULE_TXS;
extern const int MODULE_BALANCE;
extern const int MODULE_ADDR_TXS;
extern const int MODULE_BLOCK_RAW;
extern const int MODULE_V8;
extern const int MODULE_NODE_TEST;

extern const std::string MODULE_BLOCK_STR;
extern const std::string MODULE_TXS_STR;
extern const std::string MODULE_BALANCE_STR;
extern const std::string MODULE_ADDR_TXS_STR;
extern const std::string MODULE_BLOCK_RAW_STR;
extern const std::string MODULE_V8_STR;
extern const std::string MODULE_NODE_TEST_STR;

void parseModules(const std::set<std::string> &modulesStrs);

extern Modules modules;

}

#endif // MODULES_H_
