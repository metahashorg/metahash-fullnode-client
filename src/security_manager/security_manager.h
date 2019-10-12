#ifndef __SECURITY_MANAGER_H__
#define __SECURITY_MANAGER_H__

#include <map>
#include <ctime>
#include <mutex>
#include "singleton.h"

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/asio/ip/address.hpp>

namespace ip = boost::asio::ip;

class security_manager: public singleton<security_manager>
{
    friend class singleton<security_manager>;

    struct addr_info {
        addr_info(std::time_t time, int attempts):
            last(time),
            attempt(attempts) {
        }
        std::time_t last = 0;
        int attempt = 0;
    };

public:
    bool check(const ip::address& addr);
    void mark_failed(const ip::address& addr);
    void try_reset(const ip::address& addr);

protected:
    security_manager() {}
    ~security_manager() {}

private:
    bool expired(std::time_t time, const addr_info& info);

private:
    std::map<ip::address, addr_info> m_info;
    std::mutex                       m_lock;
};

#endif // __SECURITY_MANAGER_H__
