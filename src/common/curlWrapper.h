#ifndef CURL_WRAPPER_H_
#define CURL_WRAPPER_H_

#include <mutex>
#include <string>
#include <atomic>
#include <memory>

typedef void CURL;

namespace common {

struct CurlInstance {
    friend struct Curl;
    
    CurlInstance()
        : curl(nullptr, defaultDeleter)
    {}
    
    CurlInstance(std::unique_ptr<CURL, void(*)(void*)> &&curl)
        : curl(std::move(curl))
    {}
    
    CurlInstance(CurlInstance &&second)
        : curl(std::move(second.curl))
    {}
    
    CurlInstance& operator=(CurlInstance &&second) {
        this->curl = std::move(second.curl);
        return *this;
    }
    
    std::unique_ptr<CURL, void(*)(void*)> curl;
    
private:
    
    static void defaultDeleter(void*) {
        
    }
    
    mutable std::mutex mut;
};
    
struct Curl {
public:
    
    using CurlInstance = common::CurlInstance;
    
public:
    
    static void initialize();
    
    static void destroy();
    
    static CurlInstance getInstance();
    
    static std::string request(const CurlInstance &instance, const std::string &url, const std::string &postData, const std::string &header, const std::string &password);
    
    static std::string request(const CurlInstance &instance, const std::string &url, const std::string &postData, const std::string &header, const std::string &password, int timeoutSec);
    
    static std::string request(const std::string &url, const std::string &postData, const std::string &header, const std::string &password);
    
    static std::string request(const std::string &url, const std::string &postData, const std::string &header, const std::string &password, int timeoutSec);
    
private:
    
    static std::mutex initializeMut;
    static std::atomic<bool> isInitialized;
};

}

#endif // CURL_WRAPPER_H_
