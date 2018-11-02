#pragma once

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>

#include <string>

namespace settings
{
    struct service
    {
        static bool any_conns;
        static unsigned short port;
        static int threads;
        static std::vector<std::string> access;
        static std::string torrentServer;
        
        static std::string leveldbFolder;
        static std::string blocksFolder;
        static bool validateBlocks;
    };

    struct server
    {
        static std::string tor;
        static std::string proxy;
    };

    struct system
    {
        static std::string wallet_stotage;
    };

    void read();
    void read(boost::program_options::variables_map& vm);
}
