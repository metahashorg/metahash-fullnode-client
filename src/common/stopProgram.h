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
inline bool conditionWait(const std::atomic<bool> &checkedVariable, std::condition_variable &cond, std::unique_lock<std::mutex> &lock, const Predicate &predicate, bool isTimeout) {
    while (true) {
        if (cond.wait_for(lock, 1s, predicate)) {
            return true;
        } else {
            checkStopSignal(checkedVariable);
            if (isTimeout) {
                return false;
            }
        }
    }
}

template<class Predicate>
inline void conditionWait(std::condition_variable &cond, std::unique_lock<std::mutex> &lock, const Predicate &predicate) {
    conditionWait(isStopSignalCalled, cond, lock, predicate, false);
}

template<class Predicate>
inline bool conditionWaitTimeout(std::condition_variable &cond, std::unique_lock<std::mutex> &lock, const Predicate &predicate) {
    return conditionWait(isStopSignalCalled, cond, lock, predicate, true);
}

void whileTrue();

}

#endif // STOP_PROGRAM_H_
