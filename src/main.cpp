#include <iostream>
#include <memory>

#include "http_server.h"
#include "settings/settings.h"
#include "check.h"

#include "common/stopProgram.h"
#include "common/log.h"
#include "common/Thread.h"
#include "sync/Modules.h"
#include "sync/P2P_Ips.h"
#include "sync/LevelDbOptions.h"

#include "SyncSingleton.h"

#include "nslookup.h"

#include "common/network_utils.h"
#include "StatisticsServer.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace bs = boost::system;

static std::unique_ptr<http_server> server;

void runServer() {
    try {
        common::sleep(1s);
        server = std::make_unique<http_server>(settings::service::port, settings::service::threads);
        server->run();
    } catch (const common::exception &e) {
        LOGERR << "Server run Error " << e;
    } catch (const std::exception &e) {
        LOGERR << "Server run Error " << e.what();
    } catch (const common::StopException &) {
        LOGINFO << "Server stopped";
    } catch (...) {
        LOGERR << "Server run Error: Unknown";
    }
};

int main(int argc, char* argv[])
{
    common::initializeStopProgram();
    common::configureLog("./log/", true, false, false);
    try {
        torrent_node_lib::initBlockchainUtils(torrent_node_lib::BlockVersion::V2);
        std::set<std::string> modulesStrs = {torrent_node_lib::MODULE_BLOCK_STR, torrent_node_lib::MODULE_TXS_STR, torrent_node_lib::MODULE_BALANCE_STR, torrent_node_lib::MODULE_ADDR_TXS_STR, torrent_node_lib::MODULE_BLOCK_RAW_STR};
        torrent_node_lib::parseModules(modulesStrs);
                
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",									"produce help message")
            ("any",			"accept any connections")
            ("config",		po::value<std::string>(),	"config address");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        const std::string configPath = settings::getConfigPath(vm);
        settings::read(configPath);

        const std::string bestTorrentIp = getBestIp(settings::server::torName);
        settings::server::tor = bestTorrentIp;
        
        const std::string bestProxyIp = getBestIp(settings::server::proxyName);
        settings::server::proxy = bestProxyIp;
        
        const bool isStartStatistic = !settings::statistic::statisticNetwork.empty();
        
        if (isStartStatistic) {
            const std::string thisServer = common::getHostName();
            std::unique_ptr<torrent_node_lib::Statistics> statistics = std::make_unique<torrent_node_lib::StatisticsServer>(thisServer, settings::statistic::statisticNetwork, settings::statistic::statisticGroup, settings::statistic::statisticServer, settings::statistic::latencyFile, "vc1");
            torrent_node_lib::setStatistics(std::move(statistics));
        }
                
        const std::vector<std::string> serverIps = {settings::server::tor};
        std::unique_ptr<torrent_node_lib::P2P> p2p = std::make_unique<torrent_node_lib::P2P_Ips>(serverIps, 2);
        
        if (settings::system::useLocalDatabase) {
            syncSingleton() = std::make_unique<torrent_node_lib::Sync>(
                settings::system::blocksFolder, 
                torrent_node_lib::LevelDbOptions(8, true, true, settings::system::leveldbFolder, 100),
                torrent_node_lib::Sync::CachesOptions(0, 1, 100),
                p2p.get(), false, settings::system::validateBlocks
            );
        }
        
        common::Thread runServerThread(runServer);
        
        if (isStartStatistic) {
            torrent_node_lib::startStatistics();
        }
        
        if (settings::system::useLocalDatabase) {
            syncSingleton()->synchronize(2, true);
        }
        
        runServerThread.join();
        
        if (isStartStatistic) {
            torrent_node_lib::joinStatistics();
        }
        
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        LOGERR << e.what();
        return EXIT_FAILURE;
    } catch (const std::string& e) {
        LOGERR << e;
        return EXIT_FAILURE;
    }
}
