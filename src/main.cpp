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
#include "extensions/tracking_history.h"
#include "cmake_modules/GitSHA1.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace bs = boost::system;

std::unique_ptr<http_server> g_server;
ext::tracking_history g_track_his;

void signal_catcher(int sig);

void runServer() {
    try {
        common::sleep(1s);
        g_server = std::make_unique<http_server>(settings::service::port, settings::service::threads);
        g_server->run();
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
//    {
//        // Termination Signals
//        signal(SIGTERM, signal_catcher);
//        signal(SIGINT, signal_catcher);
//        signal(SIGQUIT, signal_catcher);
//        signal(SIGKILL, signal_catcher);
//        signal(SIGHUP, signal_catcher);

//        // Program Error Signals
//        signal(SIGFPE, signal_catcher);
//        signal(SIGILL, signal_catcher);
//        signal(SIGSEGV, signal_catcher);
//        signal(SIGBUS, signal_catcher);
//        signal(SIGABRT, signal_catcher);
//        signal(SIGIOT, signal_catcher);
//        signal(SIGTRAP, signal_catcher);
//        signal(SIGSYS, signal_catcher);
//    }

    common::initializeStopProgram();
    common::configureLog("./log/", true, false, false, true);
    try {
        LOGINFO << "Revision: " << g_GIT_SHA1;
        LOGINFO << "Build DateTime: " << g_GIT_DATE;

        torrent_node_lib::initBlockchainUtils(torrent_node_lib::BlockVersion::V2);
        std::set<std::string> modulesStrs = {
            torrent_node_lib::MODULE_BLOCK_STR,
            torrent_node_lib::MODULE_TXS_STR,
            torrent_node_lib::MODULE_BALANCE_STR,
            torrent_node_lib::MODULE_ADDR_TXS_STR,
            torrent_node_lib::MODULE_BLOCK_RAW_STR};
        torrent_node_lib::parseModules(modulesStrs);
                
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",                                    "produce help message")
            ("any",         "accept any connections")
            ("config",      po::value<std::string>(),   "config address");

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
        settings::server::set_tor(bestTorrentIp);
        
        const std::string bestProxyIp = getBestIp(settings::server::proxyName);
        settings::server::set_proxy(bestProxyIp);
        
        const bool isStartStatistic = !settings::statistic::statisticNetwork.empty();
        
        if (isStartStatistic) {
            const std::string thisServer = common::getHostName();
            std::unique_ptr<torrent_node_lib::Statistics> statistics = std::make_unique<torrent_node_lib::StatisticsServer>(
                        thisServer, settings::statistic::statisticNetwork, settings::statistic::statisticGroup,
                        settings::statistic::statisticServer, settings::statistic::latencyFile, "vc1");
            torrent_node_lib::setStatistics(std::move(statistics));
        }
                
        const std::vector<std::string> serverIps = {settings::server::get_tor()};
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

        if (settings::extensions::use_tracking_history) {
            if (g_track_his.init()) {
                g_track_his.run();
            }
        }

        common::Thread ip_lookup(lookup_best_ip);

        runServerThread.join();

        g_track_his.stop();
        ip_lookup.join();

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

#include <signal.h>
#include <execinfo.h>
void signal_catcher(int sig)
{
    std::string out;
    out.reserve(512);
    out.append("Caught signal \"");
    out.append(std::to_string(sig));
    out.append("\" : ");
    out.append(strsignal(sig));

    void* addrlist[40];
    int size = backtrace(addrlist, sizeof(addrlist)/sizeof(void*));

    if (size != 0) {
        out.append("\nStack trace:\n");
        char** symbollist = backtrace_symbols(addrlist, size);
        for (int i = 0; i < size; ++i) {
            out.append("  ");
            out.append(symbollist[i]);
            out.append("\n");
        }
        free(symbollist);
    }
    LOGERR << out.c_str();
}
