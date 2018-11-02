#ifndef P2P_H_
#define P2P_H_

#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <mutex>

struct CurlException {
    
    CurlException(const std::string &message)
        : message(message)
    {}
    
    const std::string message;
    
};

using BroadcastResult = std::function<void(const std::string &server, const std::string &result, const std::optional<CurlException> &exception)>;

using MakeQsAndPostFunction = std::function<std::pair<std::string, std::string>(size_t fromByte, size_t toByte)>;

struct ResponseParse {
    std::string response;
    std::optional<std::string> error;
};

using ResponseParseFunction = std::function<ResponseParse(const std::string &result)>;

class P2P {   
public:
    
    virtual ~P2P() = default;
    
    /**
     *c Выполняет запрос по всем серверам. Результаты возвращает в callback.
     *c callback должен быть готов к тому, что его будут вызывать из нескольких потоков.
     */
    virtual void broadcast(const std::string &qs, const std::string &postData, const std::string &header, const BroadcastResult &callback) const = 0;
    
    virtual std::string request(size_t responseSize, const MakeQsAndPostFunction &makeQsAndPost, const std::string &header, const ResponseParseFunction &responseParse, const std::vector<std::string> &hintsServers) const = 0;
    
    virtual std::string runOneRequest(const std::string &server, const std::string &qs, const std::string &postData, const std::string &header) const = 0;
    
protected:
    
    struct Server {
        std::string server;
        
        explicit Server(const std::string &server)
            : server(server)
        {}
    };
    
    struct Segment {
        size_t fromByte;
        size_t toByte;
        size_t posInArray;
        
        Segment() = default;
        
        Segment(size_t fromByte, size_t toByte, size_t posInArray)
            : fromByte(fromByte)
            , toByte(toByte)
            , posInArray(posInArray)
        {}
    };
    
    using RequestFunction = std::function<void(size_t threadNumber, const std::string &qs, const std::string &post, const std::string &server, const Segment &segment)>;
        
    static std::vector<Segment> makeSegments(size_t countSegments, size_t size, size_t minSize);
    
    static bool process(const std::vector<std::reference_wrapper<const Server>> &requestServers, const std::vector<Segment> &segments, const MakeQsAndPostFunction &makeQsAndPost, const RequestFunction &requestFunction);
    
};

#endif // P2P_H_
