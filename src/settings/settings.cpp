#include "settings.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>
#include "../log/log.h"

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
    std::string service::torrentServer = "167.99.242.0:5795";
    std::string service::leveldbFolder = "leveldb/";
    std::string service::blocksFolder = "blocks/";
    bool service::validateBlocks = false;
    
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
        
        if (vm.count("torrent_server"))
            settings::service::torrentServer = vm["torrent_server"].as<std::string>();
        
        if (vm.count("leveldb_folder"))
            settings::service::leveldbFolder = vm["leveldb_folder"].as<std::string>();
        
        if (vm.count("blocks_folder"))
            settings::service::blocksFolder = vm["blocks_folder"].as<std::string>();
        
        if (vm.count("validate_blocks"))
            settings::service::validateBlocks = vm["validate_blocks"].as<bool>();
    }
}
