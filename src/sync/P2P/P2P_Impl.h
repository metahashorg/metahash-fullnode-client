#ifndef P2P_IMPL_H_
#define P2P_IMPL_H_

#include <string>
#include <vector>
#include <deque>

#include "duration.h"

#include "P2PStructs.h"
#include "P2PThread.h"
#include "QueueP2P.h"
#include "P2P.h"

namespace common {
struct CurlInstance;
}

namespace torrent_node_lib {

class P2P_Impl {   
public:
    
    P2P_Impl(size_t countThreads);
    
public:
    
    struct ThreadDistribution {
        size_t from;
        size_t to;
        std::string server;
        
        ThreadDistribution(size_t from, size_t to, const std::string &server)
            : from(from)
            , to(to)
            , server(server)
        {}
    };
    
public:
    
    using RequestFunctionSimple = std::function<std::string(const std::string &qs, const std::string &post, const std::string &header, const std::string &server)>;
    
    static std::vector<Segment> makeSegments(size_t countSegments, size_t size, size_t minSize);
    
    static std::string request(const common::CurlInstance &curl, const std::string &qs, const std::string &postData, const std::string &header, const std::string &server);
    
    bool process(const std::vector<ThreadDistribution> &threadsDistribution, const std::vector<Segment> &segments, const MakeQsAndPostFunction &makeQsAndPost, const ProcessResponse &processResponse);
    
    static SendAllResult process(const std::vector<std::reference_wrapper<const std::string>> &requestServers, const std::string &qs, const std::string &post, const std::string &header, const RequestFunctionSimple &requestFunction);
    
private:
    
    size_t taskId = 1;
    
    QueueP2P blockedQueue; // queue должна стоять выше по стеку чем threads, чтобы уничтожится после всех
    
    std::deque<P2PThread> threads;
};
    
} // namespace torrent_node_lib

#endif // P2P_IMPL_H_
