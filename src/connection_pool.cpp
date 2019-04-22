#include "connection_pool.h"
#include "stopProgram.h"
#include "log.h"
#include "settings/settings.h"
#include <boost/exception/all.hpp>

#define POOL_BGN try

#define POOL_END(ret) \
    catch (const common::StopException&) {\
        return ret;\
    } catch (boost::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);\
        return ret;\
    } catch (std::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();\
        return ret;\
    } catch (...) {\
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";\
        return ret;\
    }

// socket_pool
socket_pool::socket_pool(asio::io_context& ctx)
    : m_ctx(ctx)
    , m_capacity(settings::system::conn_pool_capacity)
    , m_enable(true)
{
}

socket_pool::~socket_pool()
{
    boost::system::error_code ec;
    for (auto& it: m_busy){
        ec.clear();
        it.socket.shutdown(tcp::socket::shutdown_both, ec);
        it.socket.close(ec);
    }
    for (auto& it: m_ready){
        ec.clear();
        it.socket.shutdown(tcp::socket::shutdown_both, ec);
        it.socket.close(ec);
    }
}

bool socket_pool::enable() const
{
    return m_enable;
}

void socket_pool::enable(bool value)
{
    m_enable = value;
}

void socket_pool::capacity(size_t value)
{
    m_capacity = value;
}

pool_object socket_pool::checkout(const std::string& host)
{
    POOL_BGN
    {
        if (!m_enable) {
            throw std::runtime_error("Connection pool disabled");
        }
        std::lock_guard<std::mutex> lock(m_lock);
        std::list<ep_descr>::iterator it = m_ready.begin();
        for (; it != m_ready.end(); ++it) {
            if (it->ep == host) {
                m_busy.splice(m_busy.begin(), m_ready, it);
                return m_busy.begin();
            }
        }
        if (it == m_ready.end() && m_capacity > (m_busy.size() + m_ready.size())) {
            m_busy.insert(m_busy.begin(), {host.c_str(), m_ctx});
            return m_busy.begin();
        }
        return m_busy.end();
    }
    POOL_END(m_busy.end())
}

void socket_pool::checkin(pool_object& value)
{
    POOL_BGN
    {
        std::lock_guard<std::mutex> lock(m_lock);
        std::list<ep_descr>::iterator it = m_busy.begin();
        for (; it != m_busy.end(); ++it) {
            if (it == value) {
                it->ttl = std::chrono::high_resolution_clock::now() + std::chrono::seconds(settings::system::conn_pool_ttl);
                m_ready.splice(m_ready.cbegin(), m_busy, it);
                break;
            }
        }
        value = m_busy.end();
    }
    POOL_END()
}

bool socket_pool::valid(const pool_object& value)
{
    return value != m_busy.end();
}

void socket_pool::run_monitor()
{
    m_thr = std::thread(thread_proc, this);
}

void socket_pool::stop_monitor()
{
    POOL_BGN
    {
        enable(false);
        m_thr.join();
    }
    POOL_END()
}

void socket_pool::thread_proc(socket_pool* param)
{
    param->routine();
}

void socket_pool::routine()
{
    POOL_BGN
    {
        std::chrono::system_clock::time_point tp;
        std::chrono::system_clock::time_point now;
        while (enable())
        {
            now = std::chrono::high_resolution_clock::now();
            tp = now + std::chrono::seconds(60);
            {
                std::lock_guard<std::mutex> lock(m_lock);
                std::vector<std::list<ep_descr>::iterator> del;
                std::list<ep_descr>::iterator it = m_ready.begin();
                for (; it != m_ready.end(); ++it) {
                    if (now > it->ttl) {
                        boost::system::error_code ec;
                        it->socket.shutdown(tcp::socket::shutdown_both, ec);
                        LOGINFO << "Connection pool monitor. Shutdown socket " << it->ep << ". Error (" << ec.value() << ") " << ec.message();

                        ec.clear();
                        it->socket.close(ec);
                        LOGINFO << "Connection pool monitor. Close socket " << it->ep << ". Error (" << ec.value() << ") " << ec.message();

                        del.push_back(it);
                    }
                }
                for (auto& it: del) {
                    m_ready.erase(it);
                }
                LOGINFO << "Connection pool monitor. Ready/Busy:  " << m_ready.size() << "/" << m_busy.size();
            }
            common::checkStopSignal();
            std::this_thread::sleep_until(tp);
        }
    }
    POOL_END()
}
