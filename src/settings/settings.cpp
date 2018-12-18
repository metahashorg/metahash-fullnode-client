#include "settings.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "../log/log.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace   pt      = boost::property_tree;
namespace	asio    = boost::asio;
namespace	ip      = boost::asio::ip;
using		tcp     = boost::asio::ip::tcp;

namespace settings
{
    // service
    bool service::any_conns = {false};
    unsigned short service::port = {9999};
    int service::threads = {4};
    std::vector<std::string> service::access;
    std::string system::leveldbFolder = "leveldb/";
    std::string system::blocksFolder = "blocks/";
    bool system::validateBlocks = false;
    bool system::useLocalDatabase = true;
    std::string statistic::statisticNetwork;
    std::string statistic::statisticGroup;
    std::string statistic::statisticServer;
    std::string statistic::latencyFile;
    
    // server
    std::string server::tor     = {"tor.net-dev.metahash.org:5795"};
    std::string server::proxy   = {"proxy.net-dev.metahash.org:9999"};

    // system
    std::string system::wallet_stotage = { boost::filesystem::current_path().append("/wallet").c_str() };

    void read(const std::string &pathToConfig) {
        pt::ptree tree;
        
        boost::filesystem::path path;
        if (pathToConfig.empty()) {
            path = boost::filesystem::current_path();
            path.append("/settings.json");
        } else {
            path = pathToConfig;
        }
        
        pt::read_json(path.c_str(), tree);

        service::port = tree.get<unsigned short>("service.port", 9999);
        service::threads = tree.get<int>("service.threads", 4);

        asio::io_context ctx;
        tcp::resolver resolver(ctx);
        boost::property_tree::ptree access;
        access = tree.get_child("service.access", access);
        for (auto &v : access)
        {
            boost::system::error_code er;
            auto eps = resolver.resolve({v.second.data(), ""}, er);
            if (er)
            {
                STREAM_LOG_WRN("Couldn't resolve " << v.second.data() << " : " << er.message());
                continue;
            }
            for (auto &e : eps)
                service::access.push_back(e.endpoint().address().to_string());
        }

        server::tor     = tree.get<std::string>("server.tor", "tor.net-dev.metahash.org:5795");
        server::proxy   = tree.get<std::string>("server.proxy", "proxy.net-dev.metahash.org:9999");
        
        system::wallet_stotage = tree.get<std::string>("system.wallets-storage", boost::filesystem::current_path().append("/wallet").c_str());
                    
        settings::system::leveldbFolder = tree.get<std::string>("system.leveldb_folder");
        
        settings::system::blocksFolder = tree.get<std::string>("system.blocks_folder");
        
        settings::system::validateBlocks = tree.get<bool>("system.validate_blocks");
        
        settings::system::useLocalDatabase = tree.get<bool>("system.use_local_database");
        
        if (tree.find("statistic") != tree.not_found()) {
            settings::statistic::statisticNetwork = tree.get<std::string>("statistic.network");
            settings::statistic::statisticGroup = tree.get<std::string>("statistic.group");
            settings::statistic::statisticServer = tree.get<std::string>("statistic.server");
            settings::statistic::latencyFile = tree.get<std::string>("statistic.latency_file");
        }
    }

    std::string getConfigPath(boost::program_options::variables_map& vm) {
        if (vm.count("any")) {
            settings::service::any_conns = true;
        }
        
        if (vm.count("config")) {
            return vm["config"].as<std::string>();
        }
        return "";
    }
}
