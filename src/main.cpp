#include <iostream>

#include "http_server.h"
#include "settings/settings.h"
#include "log/log.h"

#include "common/stopProgram.h"

namespace po = boost::program_options;
namespace bs = boost::system;

static std::unique_ptr<http_server> server;

int main(int argc, char* argv[])
{
    initializeStopProgram();
    try
    {
        settings::read();

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",									"produce help message")
            ("threads",		po::value<int>(),			"number of threads")
            ("port",		po::value<unsigned short>(),"service port, default is 9999")
            ("tor",			po::value<std::string>(),	"torrent address")
            ("proxy",		po::value<std::string>(),	"proxy address")
            ("storage",		po::value<std::string>(),	"storage of wallets")
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

        server = std::make_unique<http_server>(settings::service::port, settings::service::threads);
        server->run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        STREAM_LOG_ERR(e.what());
        return EXIT_FAILURE;
    }
}
