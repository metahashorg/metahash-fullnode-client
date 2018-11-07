#ifndef PARALLEL_FOR_H_
#define PARALLEL_FOR_H_

#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <optional>

#include "check.h"
#include "log.h"
#include "stopProgram.h"

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
    
    std::optional<std::string> errorStr;
    std::mutex mutError;
    auto workerThrowWrapper = [&worker, &errorStr, &mutError](size_t threadNumber, Iter from, Iter to) {
        try {
            for (Iter iter = from; iter != to; iter++) {
                if constexpr (std::is_invocable_r<void, decltype(worker), size_t, decltype(*iter)>::value) {
                    worker(threadNumber, *iter);
                } else {
                    worker(*iter);
                    (void)threadNumber;
                }
            }
        } catch (const exception &e) {
            std::lock_guard<std::mutex> lock(mutError);
            errorStr = e;
        } catch (const StopException&) {
            return;
        } catch (...) {
            std::lock_guard<std::mutex> lock(mutError);
            errorStr = "Unknown error";
        }
    };
    
    size_t prevPosition = 0;
    auto prevIterator = begin;
    std::vector<std::thread> threads;
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
    
    for (std::thread &th: threads) {
        th.join();
    }
    CHECK(!errorStr.has_value(), errorStr.value());
}

}

#endif // PARALLEL_FOR_H_
