#ifndef STATISTICS_SERVER_H_
#define STATISTICS_SERVER_H_

#include "Statistics.h"

#include "BlockInfo.h"

#include <map>
#include <thread>
#include <mutex>
#include <atomic>

namespace torrent_node_lib {

class StatisticsServer: public torrent_node_lib::Statistics {
public:
    
    struct RequestsStat {
        std::map<std::string, size_t> requests;
        
        size_t allRequests = 0;
        
        void incRequest(const std::string &request) {
            requests[request]++;
            allRequests++;
        }
        
        void clear() {
            allRequests = 0;
            requests.clear();
        }
    };
    
private:
    
    struct BlockTimesWrapper {
        milliseconds timeGet = 0ms;
        milliseconds timeSave = 0ms;
        milliseconds timeAll = 0ms;
    };
    
    struct BlockStatistic {
        torrent_node_lib::BlockHeader bh;
        BlockTimesWrapper bt;
        torrent_node_lib::TransactionStatistics txSs;
        
        std::optional<time_point_system> tp;
    };
    
public:
    
    StatisticsServer(const std::string &serverStatName, const std::string &network, const std::string &group, const std::string &server, const std::string &latencyFile, const std::string &version);
    
    void start() override;
    
    void join() override;
    
    void abort() override;
    
    ~StatisticsServer() override;
    
public:
    
    void saveNewBlock(const torrent_node_lib::BlockInfo &bi) override;
    
    void setLastBlock(const torrent_node_lib::BlockHeader &bh) override;
    
    void incRequest(const std::string &request) override;
    
private:
    
    void processThread();
    
    size_t getLatency();
    
    static BlockStatistic makeBlockStatistic(const torrent_node_lib::BlockInfo &bi);
    
private:
    
    const std::string serverStatName;
    const std::string network;
    const std::string group;
    const std::string server;
    const std::string latencyFile;
    const std::string version;
    
    std::atomic<bool> aborted = false;
    
    std::thread workThread;
    
    mutable std::mutex blockMutex;
    BlockStatistic lastBlock;
    
    RequestsStat requestStat;
    mutable std::mutex requestStatMutex;
    
    std::string thisIp;
};

}

#endif // STATISTICS_SERVER_H_
