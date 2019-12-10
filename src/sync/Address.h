#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <string>
#include <vector>

namespace torrent_node_lib {
    
class Address {
public:
    
    const static std::string INITIAL_WALLET_TRANSACTION;
    
public:
    
    Address() = default;
    
    explicit Address(const std::vector<unsigned char> &address, bool isBlockedAddress = false);
    
    template<typename Iterator>
    Address(Iterator begin, Iterator end)
        : address(begin, end)
        , isSet(true)
    {}
    
    bool operator==(const Address &second) const;
    
    bool operator!=(const Address &second) const {
        return !(*this == second);
    }
    
    bool operator<(const Address &second) const;
    
    explicit Address(const std::string &hexAddress);
    
    const std::string& getBinaryString() const;
    
    const std::string& toBdString() const;
    
    std::string calcHexString() const;
    
    void setEmpty();
    
    bool isEmpty() const;
    
    bool isSet_() const {
        return isSet;
    }
    
    bool isInitialWallet() const;
    
    bool isScriptAddress() const;
    
    bool isTokenAddress() const;
    
private:
    
    std::string address;
    
    bool isInitialW = false;
    
    bool isSet = false;
    
};
    
}

#endif // ADDRESS_H_
