#ifndef TIME_DURATION_H_
#define TIME_DURATION_H_

#include "boost/date_time/posix_time/posix_time.hpp"

namespace utils {

class time_duration {
public:
    time_duration(bool _start);
    time_duration(bool _start, std::string message);
    ~time_duration();
    
    void start();
    void stop();
    
    void set_message(const std::string& msg) { m_msg = msg; }
    
protected:
    bool                        m_run;
    std::string                 m_msg;
    boost::posix_time::ptime    m_start;
};

}

#endif // TIME_DURATION_H_
