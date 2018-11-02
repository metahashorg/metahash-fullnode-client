#ifndef P2P_IPS_H_
#define P2P_IPS_H_

#include "P2P.h"

class P2P_Ips: public P2P {   
public:
    
    P2P_Ips(const std::vector<std::string> &servers, size_t countConnections);
    
    /**
     * c Выполняет запрос по всем серверам. Результаты возвращает в callback.
     *c callback должен быть готов к тому, что его будут вызывать из нескольких потоков.
     */
    void broadcast(const std::string &qs, const std::string &postData, const std::string &header, const BroadcastResult &callback) const override;
    
    std::string request(size_t responseSize, const MakeQsAndPostFunction &makeQsAndPost, const std::string &header, const ResponseParseFunction &responseParse, const std::vector<std::string> &hintsServers) const override;
    
    std::string runOneRequest(const std::string &server, const std::string &qs, const std::string &postData, const std::string &header) const override;
   
private:
    
    std::vector<std::reference_wrapper<const Server>> getServersList() const;
    
    std::string request(const std::string &qs, const std::string &postData, const std::string &header, const std::string &server) const;
    
private:
    
    std::vector<Server> servers;
    
    size_t countConnections;
    
};

#endif // P2P_IPS_H_
