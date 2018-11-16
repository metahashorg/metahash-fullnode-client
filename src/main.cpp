#include <iostream>
#include <memory>

#include "http_server.h"
#include "settings/settings.h"
#include "log/log.h"

#include "common/stopProgram.h"
#include "common/log.h"
#include "sync/Modules.h"
#include "sync/P2P_Ips.h"

#include "SyncSingleton.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace bs = boost::system;

static std::unique_ptr<http_server> server;

void runServer() {
    common::sleep(1s);
    server = std::make_unique<http_server>(settings::service::port, settings::service::threads);
    server->run();
};

int main(int argc, char* argv[])
{
    common::initializeStopProgram();
    common::configureLog(log4cpp::Priority::DEBUG, "./log/", true);
    try {
        torrent_node_lib::initBlockchainUtils(torrent_node_lib::BlockVersion::V2);
        std::set<std::string> modulesStrs = {torrent_node_lib::MODULE_BLOCK_STR, torrent_node_lib::MODULE_TXS_STR, torrent_node_lib::MODULE_BALANCE_STR, torrent_node_lib::MODULE_ADDR_TXS_STR, torrent_node_lib::MODULE_BLOCK_RAW_STR};
        torrent_node_lib::parseModules(modulesStrs);
                
        settings::read();

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",									"produce help message")
            ("threads",		po::value<int>(),			"number of threads")
            ("port",		po::value<unsigned short>(),"service port, default is 9999")
            ("tor",			po::value<std::string>(),	"torrent address")
            ("proxy",		po::value<std::string>(),	"proxy address")
            ("any",                                     "accept any connections");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        settings::read(vm);

        const std::vector<std::string> serverIps = {settings::system::torrentServer};
        std::unique_ptr<torrent_node_lib::P2P> p2p = std::make_unique<torrent_node_lib::P2P_Ips>(serverIps, 2);
        
        if (settings::system::useLocalDatabase) {
            syncSingleton() = std::make_unique<torrent_node_lib::Sync>(
                settings::system::blocksFolder, 
                torrent_node_lib::Sync::LevelDbOptions(8, true, true, settings::system::leveldbFolder, 100),
                torrent_node_lib::Sync::CachesOptions(0, 1, 100),
                p2p.get(), false, settings::system::validateBlocks
            );
        }
        
        common::Thread runServerThread(runServer);
        
        if (settings::system::useLocalDatabase) {
            syncSingleton()->synchronize(2, true);
        }
        
        runServerThread.join();
        
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        STREAM_LOG_ERR(e.what());
        return EXIT_FAILURE;
    }
}
