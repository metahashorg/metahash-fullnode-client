#ifndef SYNCHRONIZE_BLOCKCHAIN_H_
#define SYNCHRONIZE_BLOCKCHAIN_H_

#include "OopUtils.h"

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <unordered_map>
#include <variant>

#include "ConfigOptions.h"

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
struct Token;
struct SignBlockInfo;
struct SignTransactionInfo;
struct MinimumSignBlockHeader;
struct CommonMimimumBlockHeader;
struct RejectedTxsBlockInfo;

struct TransactionsFilters;

class P2P;

class SyncImpl;

class Sync: public common::no_copyable, common::no_moveable {    
public:
    
    Sync(const std::string &folderPath, const std::string &technicalAddress, const LevelDbOptions &leveldbOpt, const CachesOptions &cachesOpt, const GetterBlockOptions &getterBlocksOpt, const std::string &signKeyName, const TestNodesOptions &testNodesOpt, bool validateStates);
       
    void setLeveldbOptScript(const LevelDbOptions &leveldbOptScript);
    
    void setLeveldbOptNodeTest(const LevelDbOptions &leveldbOpt);
    
    const BlockChainReadInterface & getBlockchain() const;
    
    ~Sync();
    
public:
    
    static std::variant<std::monostate, BlockInfo, SignBlockInfo, RejectedTxsBlockInfo> parseBlockDump(const std::string &binaryDump, bool isValidate);
    
public:
    
    void synchronize(int countThreads);
    
    std::vector<TransactionInfo> getTxsForAddress(const Address &address, size_t from, size_t count, size_t limitTxs) const;

    std::vector<TransactionInfo> getTxsForAddress(const Address &address, size_t &from, size_t count, size_t limitTxs, const TransactionsFilters &filters) const;
    
    std::optional<TransactionInfo> getTransaction(const std::string &txHash) const;

    Token getTokenInfo(const Address &address) const;
    
    BalanceInfo getBalance(const Address &address) const;

    std::string getBlockDump(const CommonMimimumBlockHeader &bh, size_t fromByte, size_t toByte, bool isHex, bool isSign) const;

    BlockInfo getFullBlock(const BlockHeader &bh, size_t beginTx, size_t countTx) const;

    std::vector<TransactionInfo> getLastTxs() const;

    size_t getKnownBlock() const;
        
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
    
    std::string signTestString(const std::string &str, bool isHex) const;
    
    bool verifyTechnicalAddressSign(const std::string &binary, const std::vector<unsigned char> &signature, const std::vector<unsigned char> &pubkey) const;
    
    std::vector<Address> getRandomAddresses(size_t countAddresses) const;
    
    std::vector<SignTransactionInfo> findSignBlock(const BlockHeader &bh) const;
    
    std::vector<MinimumSignBlockHeader> getSignaturesBetween(const std::optional<std::vector<unsigned char>> &firstBlock, const std::optional<std::vector<unsigned char>> &secondBlock) const;
    
    std::optional<MinimumSignBlockHeader> findSignature(const std::vector<unsigned char> &hash) const;
    
private:
    
    std::unique_ptr<SyncImpl> impl;
    
};

void initBlockchainUtils();

}

#endif // SYNCHRONIZE_BLOCKCHAIN_H_
