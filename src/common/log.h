#ifndef LOG_H_
#define LOG_H_

#include "logger/easylogging++.h"

#include <string>

namespace common {
    
void configureLog(const std::string &folder, bool isAppend, bool isConsole, bool isAutoSpacing, bool isTime);

void flushLogsAll();

struct EmptyLog {
    template<typename T>
    EmptyLog& operator <<(const T &) {
        return *this;
    }
};

}

#define LOGERR LOG(ERROR)
#define LOGWARN LOG(WARNING)
#define LOGINFO LOG(INFO)
#define LOGDEBUG LOG(DEBUG)

#endif // LOG_H_
