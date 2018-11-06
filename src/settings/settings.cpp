#include "settings.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include "../log/log.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>

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
    std::string system::torrentServer = "172.104.224.65:5795";
    std::string system::leveldbFolder = "leveldb/";
    std::string system::blocksFolder = "blocks/";
    bool system::validateBlocks = false;
    bool system::useLocalDatabase = true;
    
    // server
    std::string server::tor     = {"tor.net-dev.metahash.org:5795"};
    std::string server::proxy   = {"proxy.net-dev.metahash.org:9999"};

    // system
    std::string system::wallet_stotage = { boost::filesystem::current_path().append("/wallet").c_str() };

    void read()
    {
        try
        {
            pt::ptree tree;
            auto path = boost::filesystem::current_path();
            path.append("/settings.json");
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
                       
            settings::system::torrentServer = tree.get<std::string>("system.torrent_server");
            
            settings::system::leveldbFolder = tree.get<std::string>("system.leveldb_folder");
            
            settings::system::blocksFolder = tree.get<std::string>("system.blocks_folder");
            
            settings::system::validateBlocks = tree.get<bool>("system.validate_blocks");
            
            settings::system::useLocalDatabase = tree.get<bool>("system.use_local_database");
        } catch (std::exception& e)
        {
            STREAM_LOG_ERR("Failed on read settings: " << e.what());
        }
    }

    void read(boost::program_options::variables_map& vm)
    {
        if (vm.count("any"))
            settings::service::any_conns = true;

        if (vm.count("threads"))
            settings::service::threads = std::max(vm["threads"].as<int>(), 1);

        if (vm.count("port"))
            settings::service::port = vm["port"].as<unsigned short>();

        if (vm.count("tor"))
            settings::server::tor = vm["tor"].as<std::string>();

        if (vm.count("proxy"))
            settings::server::proxy = vm["proxy"].as<std::string>();
        
        if (vm.count("storage"))
            settings::system::wallet_stotage = vm["storage"].as<std::string>();
    }
}
