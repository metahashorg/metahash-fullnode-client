#ifndef THREAD_H_
#define THREAD_H_

#include <thread>

namespace common {

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
        if (t.joinable()) {
            t.join();
        }
    }
    
};

using Thread = ThreadImpl<true>;

}

#endif // THREAD_H_
