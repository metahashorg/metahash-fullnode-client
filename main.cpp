#include <iostream>

#include "http_server.h"
#include "settings/settings.h"

namespace po = boost::program_options;
namespace bs = boost::system;

int main(int argc, char* argv[])
{
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

        http_server srv(settings::service::port, settings::service::threads);
        srv.run();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
