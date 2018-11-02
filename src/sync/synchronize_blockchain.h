#ifndef SYNCHRONIZE_BLOCKCHAIN_H_
#define SYNCHRONIZE_BLOCKCHAIN_H_

#include "OopUtils.h"

#include <string>
#include <vector>
#include <memory>
#include <set>

class BlockChainReadInterface;
struct Address;
struct TransactionInfo;
struct BlockHeader;
struct BlockInfo;
struct BalanceInfo;

class P2P;

class SyncImpl;

class Sync: public no_copyable, no_moveable {
public:
    
    struct LevelDbOptions {
        size_t writeBufSizeMb;
        bool isBloomFilter;
        bool isChecks;
        std::string_view folderName;
        size_t lruCacheMb;
        
        LevelDbOptions(size_t writeBufSizeMb, bool isBloomFilter, bool isChecks, std::string_view folderName, size_t lruCacheMb)
            : writeBufSizeMb(writeBufSizeMb)
            , isBloomFilter(isBloomFilter)
            , isChecks(isChecks)
            , folderName(folderName)
            , lruCacheMb(lruCacheMb)
        {}
    };
    
    struct CachesOptions {
        size_t maxCountElementsBlockCache;
        size_t maxCountElementsTxsCache;
        size_t macLocalCacheElements;
        
        CachesOptions(size_t maxCountElementsBlockCache, size_t maxCountElementsTxsCache, size_t macLocalCacheElements)
            : maxCountElementsBlockCache(maxCountElementsBlockCache)
            , maxCountElementsTxsCache(maxCountElementsTxsCache)
            , macLocalCacheElements(macLocalCacheElements)
        {}
    };
    
public:
    
    Sync(const std::string &folderPath, const LevelDbOptions &leveldbOpt, const CachesOptions &cachesOpt, P2P *p2p, bool getBlocksFromFile, bool isValidate);
    
    Sync(const std::string &folderPath, const LevelDbOptions &leveldbOpt, const LevelDbOptions &leveldbOptScript, const CachesOptions &cachesOpt, P2P *p2p, bool getBlocksFromFile, bool isValidate);
    
    const BlockChainReadInterface & getBlockchain() const;
    
    ~Sync();
    
public:
    
    void synchronize(int countThreads, bool isSync);

    void addUsers(const std::set<Address> &addresses);
    
    std::vector<TransactionInfo> getTxsForAddress(const Address &address, bool isTxHash, size_t from, size_t count) const;

    TransactionInfo getTransaction(const std::string &txHash) const;

    BalanceInfo getBalance(const Address &address) const;

    std::string getBlockDump(const BlockHeader &bh, size_t fromByte, size_t toByte, bool isHex) const;

    BlockInfo getFullBlock(const BlockHeader &bh, size_t beginTx, size_t countTx) const;

    std::vector<TransactionInfo> getLastTxs() const;

    size_t getKnownBlock() const;
    
private:
    
    std::unique_ptr<SyncImpl> impl;
    
};

enum class BlockVersion {
    V1, V2
};

void initBlockchainUtils(const BlockVersion &blockVersion);

#endif // SYNCHRONIZE_BLOCKCHAIN_H_