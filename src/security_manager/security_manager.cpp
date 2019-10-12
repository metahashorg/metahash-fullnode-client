#include "security_manager.h"
#include <iostream>

bool security_manager::check(const ip::address& addr)
{
    auto it = m_info.find(addr);
    if (it != m_info.end() && it->second.attempt > max_attempts) {
        std::time_t cur_time = std::time(nullptr);
        if (cur_time == static_cast<std::time_t>(-1)) {
            return true;
        }
        if (!expired(cur_time, it->second)) {
            return false;
        }
        it->second.last = cur_time;
    }
    return true;
}

void security_manager::mark_failed(const ip::address& addr)
{
    std::lock_guard<std::mutex> locker(m_lock);
    auto it = m_info.find(addr);
    if (it != m_info.end()) {
        it->second.attempt++;
    } else {
        m_info.try_emplace(addr, std::time(nullptr), 1);
    }
}

void security_manager::try_reset(const ip::address& addr)
{
    std::lock_guard<std::mutex> locker(m_lock);
    auto it = m_info.find(addr);
    if (it != m_info.end()) {
        m_info.erase(it);
    }
}

bool security_manager::expired(std::time_t time, const addr_info& info)
{
    return time - info.last > (info.attempt - max_attempts) * 300;
}
