#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <list>
#include <mutex>
#include <thread>

#define BOOST_ERROR_CODE_HEADER_ONLY
//#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace   asio    = boost::asio;
namespace   ip      = boost::asio::ip;
using       tcp     = boost::asio::ip::tcp;

struct ep_descr
{
    ep_descr(const char* host, asio::io_context& ctx)
        : ep(host)
        , socket(ctx)
    {
    }
    std::string ep;
    tcp::socket socket;
    std::chrono::system_clock::time_point ttl;
};

using pool_object = std::list<ep_descr>::iterator;

class socket_pool
{
public:
    socket_pool(asio::io_context& ctx);
    ~socket_pool();

    bool enable() const;
    void enable(bool value);

    void capacity(size_t value);

    pool_object checkout(const std::string& host);
    void checkin(pool_object& value);

    bool valid(const pool_object& value);

    void run_monitor();
    void stop_monitor();

protected:
    static void thread_proc(socket_pool* param);
    void routine();

private:
    asio::io_context&   m_ctx;
    std::list<ep_descr> m_ready;
    std::list<ep_descr> m_busy;
    std::mutex          m_lock;
    std::thread         m_thr;
    size_t              m_capacity;
    bool                m_enable;
};

#endif // CONNECTION_POOL_H
