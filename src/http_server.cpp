#include <boost/bind.hpp>
#include "http_server.h"
#include "http_session.h"
#include "settings/settings.h"
#include <boost/exception/all.hpp>
#include "log.h"
#include "common/stopProgram.h"
#include "connection_pool.h"
#include "cache/blocks_cache.h"
#include "cache/history_cache.h"

std::unique_ptr<socket_pool> g_conn_pool;
std::unique_ptr<blocks_cache> g_cache;
std::unique_ptr<history_cache> g_hist_cache;


http_server::http_server(unsigned short port /*= 9999*/, int thread_count /*= 4*/)
    : m_thread_count(thread_count)
    , m_io_ctx(m_thread_count)
    , m_run(false)
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
    } catch (const common::StopException&) {
        stop();
    }
}

bool http_server::runnig()
{
    return m_run;
}

void http_server::run()
{
    tcp::acceptor acceptor(m_io_ctx, m_ep, true);

    // Implements a custom socket option that determines whether or not an accept operation is permitted
    // to fail with boost::asio::error::connection_aborted. By default the option is false.
    // TODO check this
    //boost::asio::socket_base::enable_connection_aborted option(true);
    //acceptor.set_option(option);

    accept(acceptor);

    checkTimeoutTimer.async_wait(std::bind(&http_server::checkTimeout, this));
    
    g_conn_pool = std::make_unique<socket_pool>();
    g_cache = std::make_unique<blocks_cache>();
    g_hist_cache = std::make_unique<history_cache>();

    m_run = true;
    std::vector<std::unique_ptr<std::thread> > threads;
    for (int i = 0; i < m_thread_count; ++i)
    {
        threads.emplace_back(new std::thread(worker_proc, this));
    }

    LOGINFO << "Service runing at " << m_ep.address().to_string() << ":" << m_ep.port();

    std::cout << "Service runing at " << m_ep.address().to_string() << ":" << m_ep.port() << std::endl;

    if (settings::system::conn_pool_enable) {
        g_conn_pool->run_monitor();
    }

    if (settings::system::blocks_cache_enable) {
        g_cache->start();
    }

    if (settings::system::history_cache_enable) {
        g_hist_cache->start();
    }

    for (std::size_t i = 0; i < threads.size(); ++i) {
        threads[i]->join();
    }

    m_run = false;
    LOGINFO << "Service stoped";

    g_cache->stop();
    g_hist_cache->stop();
    g_conn_pool->stop_monitor();
}

void http_server::stop()
{
    m_run = false;
    m_io_ctx.stop();
}

void http_server::accept(tcp::acceptor& acceptor)
{
    acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket)
    {
        if (ec) {
            LOGERR << __func__ << " Failed on accept: " << ec.message();
        }
        else {
            boost::system::error_code er;
            const tcp::endpoint& ep = socket.remote_endpoint(er);
            if (er) {
                LOGERR << "Accept: Could not get remote endpoint: " << er.message();
                er.clear();
                socket.shutdown(tcp::socket::shutdown_both, er);
                socket.close(er);
            } else {
                if (check_access(ep)) {
                    std::make_shared<http_session>(std::move(socket))->run();
                } else {
                    LOGINFO << __func__ << " Reject connection " << ep.address().to_string() << ":" << ep.port();
                    socket.shutdown(tcp::socket::shutdown_both, er);
                    socket.close(er);
                }
            }
        }
        accept(acceptor);
    });
}

bool http_server::check_access(const tcp::endpoint& ep)
{
    if (settings::service::any_conns) {
        return true;
    }

    if (ep.address().is_loopback()) {
        return true;
    }

    if (std::find(settings::service::access.begin(),
                  settings::service::access.end(),
                  ep.address().to_string()) != settings::service::access.end()) {
        return true;
    }

    return false;
}

void http_server::worker_proc(http_server* param)
{
    param->routine();
}

void http_server::routine()
{
    while (m_run) {
        try {
            boost::system::error_code ec;
            m_io_ctx.run(ec);
            if (ec) {
                LOGERR << __PRETTY_FUNCTION__ << " IO context error (" << ec.value() << "): " << ec.message();
            }
            LOGINFO << __PRETTY_FUNCTION__ << " Break";
            break;
        } catch (boost::exception& ex) {
            LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);
        } catch (std::exception& ex) {
            LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();
        } catch (...) {
            LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";
        }
        LOGINFO << __PRETTY_FUNCTION__ << " Continue";
    }
}
