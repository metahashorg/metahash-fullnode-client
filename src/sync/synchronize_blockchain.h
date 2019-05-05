#ifndef SYNCHRONIZE_BLOCKCHAIN_H_
#define SYNCHRONIZE_BLOCKCHAIN_H_

#include "OopUtils.h"

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <unordered_map>

#include "LevelDbOptions.h"

namespace torrent_node_lib {

class BlockChainReadInterface;
class Address;
struct TransactionInfo;
struct BlockHeader;
struct BlockInfo;
struct BalanceInfo;
struct DelegateState;
struct V8Details;
struct CommonBalance;
struct V8Code;
struct ForgingSums;
struct NodeTestResult;
struct NodeTestTrust;
struct NodeTestCount;
struct NodeTestExtendedStat;

class P2P;

class SyncImpl;

class Sync: public common::no_copyable, common::no_moveable {
public:
       
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
       
    void setLeveldbOptScript(const LevelDbOptions &leveldbOptScript);
    
    void setLeveldbOptNodeTest(const LevelDbOptions &leveldbOpt);
    
    const BlockChainReadInterface & getBlockchain() const;
    
    ~Sync();
    
public:
    
    static BlockInfo parseBlockDump(const std::string &binaryDump, bool isValidate);
    
public:
    
    bool isVirtualMachine() const;
    
    void synchronize(int countThreads, bool isSync);

    void addUsers(const std::set<Address> &addresses);
    
    std::vector<TransactionInfo> getTxsForAddress(const Address &address, size_t from, size_t count) const;

    TransactionInfo getTransaction(const std::string &txHash) const;

    BalanceInfo getBalance(const Address &address) const;

    std::string getBlockDump(const BlockHeader &bh, size_t fromByte, size_t toByte, bool isHex) const;

    BlockInfo getFullBlock(const BlockHeader &bh, size_t beginTx, size_t countTx) const;

    std::vector<TransactionInfo> getLastTxs() const;

    size_t getKnownBlock() const;
    
    void fillSignedTransactionsInBlock(BlockHeader &bh) const;
    
    std::vector<std::pair<Address, DelegateState>> getDelegateStates(const Address &fromAddress) const;
    
    V8Details getContractDetails(const Address &contractAddress) const;
    
    CommonBalance getCommonBalance() const;
    
    V8Code getContractCode(const Address &contractAddress) const;
    
    ForgingSums getForgingSumForLastBlock(size_t blockIndent) const;
    
    ForgingSums getForgingSumAll() const;
    
    std::pair<size_t, NodeTestResult> getLastNodeTestResult(const std::string &address) const;
    
    std::pair<size_t, NodeTestTrust> getLastNodeTestTrust(const std::string &address) const;
    
    NodeTestCount getLastDayNodeTestCount(const std::string &address) const;
    
    NodeTestCount getLastDayNodesTestsCount() const;
        
    std::vector<std::pair<std::string, NodeTestExtendedStat>> filterLastNodes(size_t countTests) const;
    
    std::pair<int, size_t> calcNodeRaiting(const std::string &address, size_t countTests) const;
    
    size_t getLastBlockDay() const;
    
private:
    
    std::unique_ptr<SyncImpl> impl;
    
};

enum class BlockVersion {
    V1, V2
};

void initBlockchainUtils(const BlockVersion &blockVersion);

}

#endif // SYNCHRONIZE_BLOCKCHAIN_H_
