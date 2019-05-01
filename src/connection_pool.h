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
    ep_descr(const char* host)
        : ep(host)
        , socket(-1)
    {
    }
    std::string ep;
    tcp::socket::native_handle_type socket;
    std::chrono::system_clock::time_point ttl;
};

using pool_object = std::list<ep_descr>::iterator;

class socket_pool
{
public:
    socket_pool();
    ~socket_pool();

    bool enable() const;
    void enable(bool value);

    void capacity(size_t value);

    pool_object checkout(const std::string& host);
    void checkin(pool_object& value);

    bool valid(const pool_object& value);

    void run_monitor();
    void stop_monitor();

    size_t get_ready_size() const;
    size_t get_busy_size() const;

protected:
    static void thread_proc(socket_pool* param);
    void routine();

    void cleanup();

private:
    std::list<ep_descr> m_ready;
    std::list<ep_descr> m_busy;
    mutable std::mutex  m_lock;
    std::thread         m_thr;
    size_t              m_capacity;
    bool                m_enable;
};

#endif // CONNECTION_POOL_H
