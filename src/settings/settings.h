#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <mutex>

namespace boost {
namespace program_options {
class variables_map;
}
}

#define ATOMIC_PROP(type, name) \
  public: \
    static void set_##name(const type& value) {\
        std::lock_guard<std::mutex> lock(_locker_##name);\
        _##name = value; }\
    static type get_##name() {\
        std::lock_guard<std::mutex> lock(_locker_##name);\
        return _##name; }\
  private: \
    static type _##name; \
    static std::mutex _locker_##name;

#define ATOMIC_PROP_IMPL(type, name) \
    type server::_##name;\
    std::mutex server::_locker_##name;

namespace settings
{
    struct service {
        static bool any_conns;
        static unsigned short port;
        static int threads;
        static std::vector<std::string> access;
        static bool keep_alive;
        static bool auth_enable;
        static std::string auth_key;
    };

    struct server {
        static std::string torName;
        static std::string proxyName;
        ATOMIC_PROP(std::string, tor);
        ATOMIC_PROP(std::string, proxy);
    };

    struct system {
        static std::string wallet_stotage;
        static unsigned int jrpc_conn_timeout;
        static unsigned int jrpc_timeout;
        static unsigned int jrpc_attempts_count;
        static std::string leveldbFolder;
        static std::string blocksFolder;
        static bool validateBlocks;
        static bool allowStateBlocks;
        static bool useLocalDatabase;
        static bool conn_pool_enable;
        static unsigned int conn_pool_ttl;
        static unsigned int conn_pool_capacity;
        static unsigned int blocks_cache_ver;
        static bool blocks_cache_enable;
        static bool blocks_cache_force;
        static unsigned int blocks_cache_init_count;
        static unsigned int blocks_cache_recv_data_size;
        static unsigned int blocks_cache_recv_count;
        static bool blocks_cache_block_verification;
        static std::vector<std::string> cores;
        static bool history_cache_enable;
        static std::vector<std::string> history_cache_addrs;
    };

    struct statistic {
        static std::string statisticNetwork;
        static std::string statisticGroup;
        static std::string statisticServer;
        static std::string latencyFile;
    };

    void read(const std::string &pathToConfig);
    
    std::string getConfigPath(boost::program_options::variables_map& vm);
}
