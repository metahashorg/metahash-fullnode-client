#ifndef LIMIT_ARRAY_H_
#define LIMIT_ARRAY_H_

#include <deque>
#include <algorithm>

namespace torrent_node_lib {

class LimitArray {
public:
    
    LimitArray(size_t limit)
        : limit(limit)
    {}
    
    void add(size_t element) {
        arr.emplace_back(element);
        if (arr.size() > limit) {
            arr.pop_front();
        }
    }
    
    size_t max() const {
        if (arr.empty()) {
            return 0;
        }
        return *std::max_element(arr.begin(), arr.end());
    }
    
    bool filled() const {
        return arr.size() == limit;
    }
    
private:
    
    size_t limit;
    
    std::deque<size_t> arr;
};

} // namespace torrent_node_lib

#endif // LIMIT_ARRAY_H_
