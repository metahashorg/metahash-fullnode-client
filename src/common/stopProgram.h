#ifndef STOP_PROGRAM_H_
#define STOP_PROGRAM_H_

#include <atomic>
#include <thread>
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

template<bool IsJoinable = true>
struct ThreadImpl {
    
    std::thread t;
    
    ThreadImpl() = default;
    
    ThreadImpl(const ThreadImpl<IsJoinable> &second) = delete;
    ThreadImpl<IsJoinable>& operator=(const ThreadImpl<IsJoinable> &second) = delete;
    
    ThreadImpl(ThreadImpl<IsJoinable> &&second) = default;
    ThreadImpl<IsJoinable>& operator=(ThreadImpl<IsJoinable> &&second) = default;
    
    template<typename... Args>
    ThreadImpl(Args&& ...args)
        : t(std::forward<Args>(args)...)
    {}
    
    ~ThreadImpl() {
        if (t.joinable()) {
            if (IsJoinable) {
                t.join();
            } else {
                t.detach();
            }
        }
    }
    
    void join() {
        t.join();
    }
    
};

using Thread = ThreadImpl<true>;

}

#endif // STOP_PROGRAM_H_
