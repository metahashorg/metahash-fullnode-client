#ifndef PARALLEL_FOR_H_
#define PARALLEL_FOR_H_

#include <functional>
#include <vector>
#include <mutex>
#include <optional>

#include "check.h"
#include "stopProgram.h"
#include "Thread.h"

namespace common {

template<typename Iter, class Operator>
inline void parallelFor(int countThreads, Iter begin, Iter end, const Operator &worker) {
    const size_t countElements = std::distance(begin, end);
    if (countElements == 0) {
        return;
    }
    countThreads = (int)std::min(size_t(countThreads), countElements);
    const size_t step = countElements / countThreads;
    CHECK(step != 0, "step == 0");
    
    std::exception_ptr error;
    std::mutex mutError;
    const auto workerThrowWrapper = [&worker, &error, &mutError](size_t threadNumber, Iter from, Iter to) {
        try {
            for (Iter iter = from; iter != to; iter++) {
                if constexpr (std::is_invocable_r<void, decltype(worker), size_t, decltype(*iter)>::value) {
                    worker(threadNumber, *iter);
                } else {
                    worker(*iter);
                    (void)threadNumber;
                }
            }
        } catch (const StopException&) {
            return;
        } catch (...) {
            std::lock_guard<std::mutex> lock(mutError);
            error = std::current_exception();
        }
    };
    
    size_t prevPosition = 0;
    auto prevIterator = begin;
    std::vector<Thread> threads;
    threads.reserve(countThreads - 1);
    for (int i = 0; i < countThreads - 1; i++) {
        const size_t nextPosition = std::min(prevPosition + step, countElements);
        auto nextIterator = prevIterator;
        std::advance(nextIterator, nextPosition - prevPosition);
        const size_t threadNumber = threads.size() + 1;
        threads.emplace_back(workerThrowWrapper, threadNumber, prevIterator, nextIterator);
        prevPosition = nextPosition;
        prevIterator = nextIterator;
    }

    workerThrowWrapper(0, prevIterator, end);
    
    for (Thread &th: threads) {
        th.join();
    }
    if (error) {
        std::rethrow_exception(error);
    }
}

}

#endif // PARALLEL_FOR_H_
