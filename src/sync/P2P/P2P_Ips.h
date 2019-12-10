#ifndef P2P_IPS_H_
#define P2P_IPS_H_

#include "P2P.h"
#include "P2P_Impl.h"

#include <map>

#include "LimitArray.h"

namespace common {
struct CurlInstance;
}

namespace torrent_node_lib {

class P2P_Ips: public P2P {   
public:
    
    P2P_Ips(const std::vector<std::string> &servers, size_t countConnections);
    
    ~P2P_Ips() override;
    
    /**
     * c Выполняет запрос по всем серверам. Результаты возвращает в callback.
     *c callback должен быть готов к тому, что его будут вызывать из нескольких потоков.
     */
    void broadcast(const std::string &qs, const std::string &postData, const std::string &header, const BroadcastResult &callback) const override;
    
    std::string request(size_t responseSize, bool isPrecisionSize, const MakeQsAndPostFunction &makeQsAndPost, const std::string &header, const ResponseParseFunction &responseParse, const std::vector<std::string> &hintsServers) override;
    
    std::vector<std::string> requests(size_t countRequests, const MakeQsAndPostFunction2 &makeQsAndPost, const std::string &header, const ResponseParseFunction &responseParse, const std::vector<std::string> &hintsServers) override;
    
    std::string runOneRequest(const std::string &server, const std::string &qs, const std::string &postData, const std::string &header) const override;
   
    SendAllResult requestAll(const std::string &qs, const std::string &postData, const std::string &header, const std::set<std::string> &additionalServers) const override;
    
private:
    
    size_t getMaxServersCount(const std::vector<std::string> &srvrs) const;
    
    std::vector<P2P_Impl::ThreadDistribution> getServersList(const std::vector<std::string> &srvrs, size_t countSegments) const;
    
    std::vector<std::string> requestImpl(size_t responseSize, size_t minResponseSize, bool isPrecisionSize, const torrent_node_lib::MakeQsAndPostFunction &makeQsAndPost, const std::string &header, const torrent_node_lib::ResponseParseFunction &responseParse, const std::vector<std::string> &hintsServers);
    
private:
    
    P2P_Impl p2p;
    
    std::vector<std::string> servers;
    
    size_t countConnections;
    
    std::vector<common::CurlInstance> curlsBroadcast;
    
};

}

#endif // P2P_IPS_H_
