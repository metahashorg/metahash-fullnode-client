#include <iostream>
#include "log.h"
#include <syslog.h>
#include <iostream>


namespace logg
{

std::mutex logger::_locker;

std::map<int, std::string> logger::pri_str = {
    {LOG_ERR,       "Error"},
    {LOG_WARNING,   "Warning"},
    {LOG_INFO,      "Info"},
    {LOG_DEBUG,     "Debug"}
};

logger::logger()
{
    ::openlog("metahash_service", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_SYSLOG);
}

logger::~logger()
{
    ::closelog();
}

void logger::push(int pri, const std::string& mess)
{
    std::lock_guard<std::mutex> guard(_locker);
    ::syslog(pri, "%s: %s", pri_str[pri].c_str(), mess.c_str());
    std::cout << pri_str[pri] << ": " << mess << std::endl;
}

void push_err(const std::string& mess)
{
    _logger.push(LOG_ERR, mess);
}

void push_wrn(const std::string& mess)
{
    _logger.push(LOG_WARNING, mess);
}

void push_inf(const std::string& mess)
{
    _logger.push(LOG_INFO, mess);
}

void push_dbg(const std::string& mess)
{
#ifdef USE_DEBUG_LOG_MESSAGE
    _logger.push(LOG_DEBUG, mess);
#endif
}

}
