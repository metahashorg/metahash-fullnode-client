#include <iostream>
#include <iomanip>
#include <memory>
#include "http_server.h"
#include "settings/settings.h"
#include "check.h"
#include "common/stopProgram.h"
#include "common/log.h"
#include "common/Thread.h"
#include "sync/Modules.h"
#include "sync/P2P/P2P_Ips.h"
#include "sync/ConfigOptions.h"
#include "SyncSingleton.h"
#include "nslookup.h"
#include "version.h"
#include "common/network_utils.h"
#include "StatisticsServer.h"
#include "cmake_modules/GitSHA1.h"
#include "json_rpc_schema.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace bs = boost::system;

void signal_catcher(int sig);

void runServer() {
    http_server srv(settings::service::port, settings::service::threads);
    try {
        common::sleep(1s);
        srv.run();
    } catch (const common::exception &e) {
        LOGERR << "Server Common Exception " << e;
    } catch (const std::exception &e) {
        LOGERR << "Server Std Exception " << e.what();
    } catch (const common::StopException &) {
        LOGINFO << "Server stopped";
    } catch (...) {
        LOGERR << "Server Unknow Exception";
    }
    srv.stop();
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
        std::cout << "Version: " << get_version() << std::endl;
        std::cout << "Revision: " << g_GIT_SHA1 << std::endl;
        std::cout << "Build Date: " << g_GIT_DATE << std::endl;

        LOGINFO << "Version: " << get_version();
        LOGINFO << "Revision: " << g_GIT_SHA1;
        LOGINFO << "Build DateTime: " << g_GIT_DATE;

        torrent_node_lib::initBlockchainUtils();
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
            ("any",                                     "accept any connections")
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

        std::vector<NsResult> ip;
        if (get_ip_addresses(settings::server::torName, ip)) {
            LOGINFO << "Torrenrs:";
            std::cout << "Torrenrs:" << std::endl;
            for (const auto& i: ip) {
                std::cout << std::left << std::setfill(' ') << std::setw(25) << i.server << i.timeout << " ms" << std::endl;
                LOGINFO << i.server << " " << i.timeout << " ms";
            }
            settings::server::set_tor(ip.begin()->server);
        } else {
            std::cout << "Could not get torrent adresses" << std::endl;
            LOGERR << "Could not get torrent adresses";
            return EXIT_SUCCESS;
        }

        ip.clear();
        if (get_ip_addresses(settings::server::proxyName, ip)) {
            LOGINFO << "Proxies:";
            std::cout << "Proxies:" << std::endl;
            for (const auto& i: ip) {
                std::cout << std::left << std::setfill(' ') << std::setw(25) << i.server << i.timeout << " ms" << std::endl;
                LOGINFO << i.server << " " << i.timeout << " ms";
            }
            settings::server::set_proxy(ip.begin()->server);
        } else {
            std::cout << "Could not get proxy adresses" << std::endl;
            LOGERR << "Could not get proxy adresses";
            return EXIT_SUCCESS;
        }
        
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
                "",
                torrent_node_lib::LevelDbOptions(8, true, true, settings::system::leveldbFolder, 100),
                torrent_node_lib::CachesOptions(0, 1, 100),
                torrent_node_lib::GetterBlockOptions(100, 100, p2p.get(), false, settings::system::validateBlocks, false, false),
                "",
                torrent_node_lib::TestNodesOptions(0, "", ""),
                false
            );
        }

        common::Thread runServerThread(runServer);

        torrent_node_lib::StatisticGuard statGuard;
        if (isStartStatistic) {
            statGuard = torrent_node_lib::startStatistics();
        }
        
        if (settings::system::useLocalDatabase) {
            syncSingleton()->synchronize(2);
        }

        common::Thread ip_lookup(lookup_best_ip);

        runServerThread.join();

        ip_lookup.join();

        statGuard.join();

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
