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

namespace po = boost::program_options;
namespace bs = boost::system;

static std::unique_ptr<http_server> server;

void runServer() {
    sleep(1s);
    server = std::make_unique<http_server>(settings::service::port, settings::service::threads);
    server->run();
};

int main(int argc, char* argv[])
{
    initializeStopProgram();
    configureConsoleLog(log4cpp::Priority::DEBUG);
    try {
        initBlockchainUtils(BlockVersion::V2);
        std::set<std::string> modulesStrs = {MODULE_BLOCK_STR, MODULE_TXS_STR, MODULE_BALANCE_STR, MODULE_ADDR_TXS_STR, MODULE_BLOCK_RAW_STR};
        parseModules(modulesStrs);
                
        settings::read();

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",									"produce help message")
            ("threads",		po::value<int>(),			"number of threads")
            ("port",		po::value<unsigned short>(),"service port, default is 9999")
            ("tor",			po::value<std::string>(),	"torrent address")
            ("proxy",		po::value<std::string>(),	"proxy address")
            ("storage",		po::value<std::string>(),	"storage of wallets")
            ("torrent_server",		po::value<std::string>(),	"server torrent")
            ("blocks_folder",		po::value<std::string>(),	"blocks folder")
            ("leveldb_folder",		po::value<std::string>(),	"leveldb folder")
            ("validate_blocks",		po::value<bool>(),	" validate blocks")
            ("use_local_database",		po::value<bool>(),	" use_local_database")
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

        const std::vector<std::string> serverIps = {settings::service::torrentServer};
        std::unique_ptr<P2P> p2p = std::make_unique<P2P_Ips>(serverIps, 2);
        
        if (settings::service::useLocalDatabase) {
            syncSingleton() = std::make_unique<Sync>(
                settings::service::blocksFolder, 
                Sync::LevelDbOptions(8, true, true, settings::service::leveldbFolder, 100),
                Sync::CachesOptions(0, 1, 100),
                p2p.get(), false, settings::service::validateBlocks
            );
        }
        
        Thread runServerThread(runServer);
        
        if (settings::service::useLocalDatabase) {
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
