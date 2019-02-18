#ifndef STOP_PROGRAM_H_
#define STOP_PROGRAM_H_

#include <atomic>
#include <mutex>
#include <condition_variable>

#include "duration.h"

namespace common {

struct StopException {
    
};

extern std::atomic<bool> isStopSignalCalled;

void initializeStopProgram();

void stopHandler(int sig);

void checkStopSignal(const std::atomic<bool> &checkedVariable);

void checkStopSignal();

void sleep(const seconds &dur);

void sleepMs(const milliseconds &dur);

template<class Predicate>
inline void conditionWait(const std::atomic<bool> &checkedVariable, std::condition_variable &cond, std::unique_lock<std::mutex> &lock, const Predicate &predicate) {
    while (true) {
        if (cond.wait_for(lock, 1s, predicate)) {
            break;
        } else {
            checkStopSignal(checkedVariable);
        }
    }
}

template<class Predicate>
inline void conditionWait(std::condition_variable &cond, std::unique_lock<std::mutex> &lock, const Predicate &predicate) {
    conditionWait(isStopSignalCalled, cond, lock, predicate);
}

void whileTrue();

}

#endif // STOP_PROGRAM_H_
