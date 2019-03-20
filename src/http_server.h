#pragma once

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/system_timer.hpp>

namespace   asio    = boost::asio;
namespace   ip      = boost::asio::ip;
using       tcp     = boost::asio::ip::tcp;

class http_server
{
public:
    http_server(unsigned short port = 9999, int thread_count = 4);
    ~http_server();

    void run();
    void stop();
    bool runnig();

protected:
    void accept(tcp::acceptor& acceptor);

    bool check_access(const tcp::endpoint& ep);

private:
    
    void checkTimeout();
    
protected:
    int                 m_thread_count;
    asio::io_context    m_io_ctx;
    tcp::endpoint       m_ep;
    bool                m_run;
    
private:
    
    boost::asio::system_timer checkTimeoutTimer;
};
