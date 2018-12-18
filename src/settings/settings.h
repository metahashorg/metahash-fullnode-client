#pragma once

#include <string>
#include <vector>

namespace boost {
namespace program_options {
class variables_map;
}
}

namespace settings
{
    struct service
    {
        static bool any_conns;
        static unsigned short port;
        static int threads;
        static std::vector<std::string> access;
    };

    struct server
    {
        static std::string tor;
        static std::string proxy;
    };

    struct system
    {
        static std::string wallet_stotage;
        
        static std::string leveldbFolder;
        static std::string blocksFolder;
        static bool validateBlocks;
        
        static bool useLocalDatabase;
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
