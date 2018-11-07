#include <boost/bind.hpp>
#include "http_server.h"
#include "http_session.h"
#include "settings/settings.h"
#include "log/log.h"

#include <iostream>

#include "common/stopProgram.h"

http_server::http_server(unsigned short port /*= 9999*/, int thread_count /*= 4*/)
    : m_thread_count(thread_count)
    , m_io_ctx(m_thread_count)
    , checkTimeoutTimer(m_io_ctx)
{
    m_ep.port(port);
}

http_server::~http_server()
{

}

void http_server::checkTimeout() {
    try {
        common::checkStopSignal();
        checkTimeoutTimer.expires_after(seconds(1));
        checkTimeoutTimer.async_wait(std::bind(&http_server::checkTimeout, this));
    } catch (const common::StopException &e) {
        stop();
    }
}

void http_server::run()
{
    tcp::acceptor acceptor(m_io_ctx, m_ep, true);
    accept(acceptor);

    checkTimeoutTimer.async_wait(std::bind(&http_server::checkTimeout, this));
    
    std::vector<std::unique_ptr<std::thread> > threads;
    for (int i = 0; i < m_thread_count; ++i)
    {
        threads.emplace_back(new std::thread(boost::bind(&boost::asio::io_context::run, &m_io_ctx)));
    }

    STREAM_LOG_INF("Service runing at " << m_ep.address().to_string() << ":" << m_ep.port())

    for (std::size_t i = 0; i < threads.size(); ++i)
    {
        threads[i]->join();
    }

    STREAM_LOG_INF("Service stoped")
}

void http_server::stop()
{
    m_io_ctx.stop();
}

void http_server::accept(tcp::acceptor& acceptor)
{
    acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket)
    {
        if (ec)
        {
            STREAM_LOG_ERR("Failed on accept: " << ec.message())
        }
        else
        {
            const tcp::endpoint& ep = socket.remote_endpoint();
            if (check_access(ep))
            {
                std::make_shared<http_session>(std::move(socket))->run();
            }
            else
            {
                STREAM_LOG_INF("Droping connection " << ep.address().to_string() << ":" << ep.port())
                socket.shutdown(tcp::socket::shutdown_both);
                socket.close();
            }
        }
        accept(acceptor);
    });
}

bool http_server::check_access(const tcp::endpoint& ep)
{

    if (settings::service::any_conns)
        return true;

    if (ep.address().is_loopback())
        return true;

    if (std::find(settings::service::access.begin(),
              settings::service::access.end(),
              ep.address().to_string()) != settings::service::access.end())
        return true;

    return false;
}
