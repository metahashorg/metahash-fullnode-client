#ifndef P2P_THREAD_H_
#define P2P_THREAD_H_

#include <thread>
#include <string>
#include <mutex>
#include <memory>

#include "ReferenceWrapper.h"
#include "P2PStructs.h"

#include "OopUtils.h"

namespace common {
struct CurlInstance;
}

namespace torrent_node_lib {

class QueueP2P;
    
class P2PThread: public common::no_copyable, public common::no_moveable {
public:
    
    P2PThread(QueueP2P &queue);
    
    ~P2PThread();
    
public:
    
    void newTask(size_t taskNum, const std::string &server, const ReferenseWrapperSlave<P2PReferences> &references, size_t countUniqueServers);
    
private:
    
    void work();
    
private:
    
    std::thread th;
    
    std::unique_ptr<common::CurlInstance> curl;
    
    QueueP2P &queue;
    
private:
    
    bool stopped = false;
    
    std::string currentServer;
    
    size_t taskNumber = 0;
    
    size_t countUniqueServers;
    
    std::unique_ptr<ReferenseWrapperSlave<P2PReferences>> referencesWrapper;
    
    std::mutex mut;
    
    std::condition_variable cond;
};
    
} // namespace torrent_node_lib

#endif // P2P_THREAD_H_
