#ifndef BLOCK_INFO_H_
#define BLOCK_INFO_H_

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <set>
#include <map>
#include <unordered_map>

#include "duration.h"

namespace torrent_node_lib {

class Address {
public:
    
    const static std::string INITIAL_WALLET_TRANSACTION;
    
public:
    
    Address() = default;
    
    explicit Address(const std::vector<unsigned char> &address, bool isBlockedAddress = false);
        
    template<typename Iterator>
    Address(Iterator begin, Iterator end);
        
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
    
private:
    
    std::string address;
    
    bool isInitialW = false;
    
    bool isSet = false;
    
};

struct FilePosition {
    size_t pos;
    std::string fileName;
    
    FilePosition() = default;
    
    FilePosition(const std::string &fileName, size_t pos)
        : pos(pos)
        , fileName(fileName)
    {}
    
    std::string serialize() const;
    
    void serialize(std::vector<char> &buffer) const;
    
    static FilePosition deserialize(const std::string &raw, size_t from, size_t &nextFrom);
    
    static FilePosition deserialize(const std::string &raw, size_t &from);
    
    static FilePosition deserialize(const std::string &raw);
    
};

struct AddressInfo {
    
    AddressInfo() = default;
    
    AddressInfo(size_t pos, const std::string &fileName, size_t blockNumber)
        : filePos(fileName, pos)
        , blockNumber(blockNumber)
    {}
    
    FilePosition filePos;
    
    size_t blockNumber = 0;
    
    std::optional<int64_t> undelegateValue;
    
    void serialize(std::vector<char> &buffer) const;
    
    static AddressInfo deserialize(const std::string &raw);
};

struct TransactionStatus {
    TransactionStatus(const std::string &txHash, size_t blockNumber)
        : transaction(txHash)
        , blockNumber(blockNumber)
    {}
    
    struct Delegate {
        void serialize(std::vector<char> &buffer) const {}
        static Delegate deserialize(const std::string &raw, size_t &fromPos) {
            return Delegate();
        }
    };
       
    struct UnDelegate {
        int64_t value;
        std::string delegateHash;
        
        UnDelegate() = default;
        
        UnDelegate(int64_t value, const std::string &delegateHash)
            : value(value)
            , delegateHash(delegateHash)
        {}
        
        void serialize(std::vector<char> &buffer) const;
        static UnDelegate deserialize(const std::string &raw, size_t &fromPos);
    };
    
    struct V8Status {
        bool isServerError = false;
        bool isScriptError = false;
        
        Address compiledContractAddress;
        
        void serialize(std::vector<char> &buffer) const;
        static V8Status deserialize(const std::string &raw, size_t &fromPos);
    };
    
    bool isSuccess;
    
    std::string transaction;
    
    size_t blockNumber;
    
    std::variant<std::monostate, Delegate, UnDelegate, V8Status> status;
    
    void serialize(std::vector<char> &buffer) const;
    
    static TransactionStatus deserialize(const std::string &raw);
    
private:
    
    TransactionStatus() = default;
    
    template <std::size_t ... I>
    void parseVarint(const std::string &raw, size_t &fromPos, size_t number, std::index_sequence<I ... >);
    
    template<size_t I>
    void tryParse(const std::string &raw, size_t &fromPos, size_t number);
};

struct TransactionInfo {
    struct DelegateInfo {
        int64_t value;
        bool isDelegate;
    };
    
    struct ScriptInfo {
        std::string txRaw;
        bool isInitializeScript;
    };
    
    std::string hash;
    Address fromAddress;
    Address toAddress;
    int64_t value;
    int64_t fees = 0;
    uint64_t nonce = 0;
    size_t blockNumber = 0;
    size_t sizeRawTx = 0;
    bool isSaveToBd = true;
    
    bool isSignBlockTx = false;
        
    std::optional<uint64_t> intStatus;
    
    std::vector<char> sign;
    std::vector<unsigned char> pubKey;
    
    std::vector<unsigned char> data;
    
    std::string allRawTx;
    
    std::optional<int64_t> realFees;
    
    std::optional<DelegateInfo> delegate;
    
    std::optional<ScriptInfo> scriptInfo;
    
    FilePosition filePos;
    
    std::optional<TransactionStatus> status;
    
    bool isModuleNotSet = false;
    
    bool isInitialized = false;
    
    void serialize(std::vector<char> &buffer) const;
    
    static TransactionInfo deserialize(const std::string &raw);
    
    static TransactionInfo deserialize(const std::string &raw, size_t &from);
    
    void calcRealFee();
    
    bool isStatusNeed() const {
        return delegate.has_value() || scriptInfo.has_value();
    }
    
    bool isIntStatusNoBalance() const;
    
    bool isIntStatusNotSuccess() const;
    
    bool isIntStatusForging() const;
    
    bool isIntStatusNodeTest() const;
    
    static std::set<uint64_t> getForgingIntStatuses();
};

struct BalanceInfo {
    struct DelegateBalance {
        size_t delegate = 0;
        size_t undelegate = 0;
        size_t delegated = 0;
        size_t undelegated = 0;
        size_t reserved = 0;
        size_t countOp = 0;
    };
    
    struct ForgedBalance {
        size_t forged = 0;
        size_t countOp = 0;
    };
    
    size_t received = 0;
    size_t spent = 0;
    size_t countReceived = 0;
    size_t countSpent = 0;
    
    std::optional<DelegateBalance> delegated;
    
    std::optional<ForgedBalance> forged;
    
    size_t blockNumber = 0;
    
    BalanceInfo() = default;
    
    void plusWithDelegate(const TransactionInfo &tx, const Address &address, const std::optional<int64_t> &undelegateValue, bool isOkStatus);
    
    void plusWithoutDelegate(const TransactionInfo &tx, const Address &address, bool changeBalance, bool isForging);
        
    BalanceInfo& operator+=(const BalanceInfo &second);
    
    void serialize(std::vector<char> &buffer) const;
    
    static BalanceInfo deserialize(const std::string &raw);
    
};

BalanceInfo operator+(const BalanceInfo &first, const BalanceInfo &second);

struct CommonBalance {
    size_t money = 0;
    size_t blockNumber = 0;
       
    CommonBalance() = default;
    
    void serialize(std::vector<char> &buffer) const;
    
    static CommonBalance deserialize(const std::string &raw);
};

struct BlockHeader {
    size_t timestamp;
    uint64_t blockSize;
    uint64_t blockType;
    std::string hash;
    std::string prevHash;
    
    std::vector<unsigned char> signature;
    
    std::string txsHash;
    
    std::optional<size_t> countTxs;
    
    FilePosition filePos;
    
    size_t endBlockPos = 0;
    
    std::optional<size_t> blockNumber;
    
    std::vector<TransactionInfo> blockSignatures;
    
    std::string serialize() const;
    
    static BlockHeader deserialize(const std::string &raw);
    
    bool isStateBlock() const;
    
    bool isSimpleBlock() const;
    
    bool isForgingBlock() const;
    
    std::string getBlockType() const;
};

struct BlockTimes {
    time_point timeBegin;
    time_point timeEnd;
    
    time_point timeBeginGetBlock;
    time_point timeEndGetBlock;
    
    time_point timeBeginSaveBlock;
    time_point timeEndSaveBlock;
};

struct TransactionStatistics {
    size_t countTransferTxs = 0;
    size_t countInitTxs = 0;
};

struct BlockInfo {
    BlockHeader header;
    
    BlockTimes times;
    
    TransactionStatistics txsStatistic;
    
    std::vector<TransactionInfo> txs;
};

struct BlocksMetadata {
    std::string blockHash;
    std::string prevBlockHash;
    size_t addressCounter = 0;
    size_t blockNumber = 0;
       
    BlocksMetadata() = default;
    
    BlocksMetadata(const std::string &blockHash, const std::string &prevBlockHash, size_t addressCounter, size_t blockNumber)
        : blockHash(blockHash)
        , prevBlockHash(prevBlockHash)
        , addressCounter(addressCounter)
        , blockNumber(blockNumber)
    {}
    
    std::string serialize() const;
       
    static BlocksMetadata deserialize(const std::string &raw);
    
};

struct ScriptBlockInfo {
    size_t blockNumber = 0;
    std::string blockHash;
    size_t countVal = 0;
    
    ScriptBlockInfo() = default;
    
    ScriptBlockInfo(size_t blockNumber, const std::string &blockHash, size_t countVal)
        : blockNumber(blockNumber)
        , blockHash(blockHash)
        , countVal(countVal)
    {}
    
    std::string serialize() const;
    
    static ScriptBlockInfo deserialize(const std::string &raw);
    
};

struct NodeStatBlockInfo {
    size_t blockNumber = 0;
    std::string blockHash;
    size_t countVal = 0;
    
    NodeStatBlockInfo() = default;
    
    NodeStatBlockInfo(size_t blockNumber, const std::string &blockHash, size_t countVal)
        : blockNumber(blockNumber)
        , blockHash(blockHash)
        , countVal(countVal)
    {}
    
    std::string serialize() const;
    
    static NodeStatBlockInfo deserialize(const std::string &raw);
    
};

struct FileInfo {
    
    FilePosition filePos;
    
    FileInfo()
        : filePos("", 0)
    {}
    
    std::string serialize() const;
    
    static FileInfo deserialize(const std::string &raw);
    
};

template<class ValueType>
struct BatchResults {
    using Address = std::string;
    
    std::vector<std::pair<Address, ValueType>> elements;
    size_t lastBlockNum;
};

BalanceInfo calcBalance(const Address &address, const std::vector<TransactionInfo> &txs);

struct DelegateState {
    int64_t value = 0;
    
    std::string hash;
    
    DelegateState() = default;
    
    DelegateState(int64_t value, const std::string &hash)
        : value(value)
        , hash(hash)
    {}
    
    void serialize(std::vector<char> &buffer) const;
    
    static DelegateState deserialize(const std::string &raw);
};

struct DelegateStateHelper {
    size_t blockNumber = 0;
    
    DelegateStateHelper() = default;
    
    DelegateStateHelper(size_t blockNumber)
        : blockNumber(blockNumber)
    {}
    
    void serialize(std::vector<char> &buffer) const;
    
    static DelegateStateHelper deserialize(const std::string &raw);
    
};

struct V8State {
    enum class ErrorType {
        OK, USER_ERROR, SERVER_ERROR, SCRIPT_ERROR
    };
    
    V8State() = default;
    
    V8State(size_t blockNumber)
        : blockNumber(blockNumber)
    {}
    
    Address address;
    std::string state;
    
    size_t blockNumber = 0;
    
    std::string details;
    
    size_t errorCode = 0;
    ErrorType errorType = ErrorType::OK;
    std::string errorMessage;
    
    std::string serialize() const;
    
    static V8State deserialize(const std::string &raw);

};

struct V8Details {
    V8Details() = default;
    
    V8Details(const std::string &details, const std::string &lastError)
        : details(details)
        , lastError(lastError)
    {}
    
    std::string details;
    
    std::string lastError;

    std::string serialize() const;
    
    static V8Details deserialize(const std::string &raw);
    
};

struct V8Code {
    V8Code() = default;
    
    V8Code(const std::vector<unsigned char> &code)
        : code(code)
    {}
    
    std::vector<unsigned char> code;
    
    std::string serialize() const;
    
    static V8Code deserialize(const std::string &raw);
    
};

struct ForgingSums {
    std::unordered_map<uint64_t, size_t> sums;
    size_t blockNumber = 0;
};

struct NodeTestType {
    enum class Type {
        unknown, torrent, proxy
    };
    
    Type type = Type::unknown;
    
    NodeTestType() = default;
    
    NodeTestType(Type type)
        : type(type)
    {}
    
    std::string serialize() const;
    
    static NodeTestType deserialize(const std::string &raw, size_t &from);
    
};

struct NodeTestResult {
    std::string result;
    size_t timestamp = 0;
    NodeTestType type;
    std::string ip;
    
    NodeTestResult() = default;
    
    NodeTestResult(const std::string &result, size_t timestamp, NodeTestType type, const std::string &ip)
        : result(result)
        , timestamp(timestamp)
        , type(type)
        , ip(ip)
    {}
    
    std::string serialize() const;
    
    static NodeTestResult deserialize(const std::string &raw);

};

struct NodeTestCount {
    
    size_t count = 0;
    
    size_t day = 0;
       
    NodeTestCount() = default;
    
    NodeTestCount(size_t count, size_t day)
        : count(count)
        , day(day)
    {}
    
    std::string serialize() const;
    
    static NodeTestCount deserialize(const std::string &raw);
    
};

struct NodeTestExtendedStat {
    
    NodeTestCount count;
    
    NodeTestType type;
    
    std::string ip;
    
    NodeTestExtendedStat() = default;
    
    NodeTestExtendedStat(const NodeTestCount &count, const NodeTestType &type, const std::string &ip)
        : count(count)
        , type(type)
        , ip(ip)
    {}
    
};

struct AllTestedNodes {
    
    std::set<std::string> nodes;
    
    size_t day = 0;
    
    AllTestedNodes() = default;
    
    AllTestedNodes(size_t day)
        : day(day)
    {}
    
    std::string serialize() const;
    
    static AllTestedNodes deserialize(const std::string &raw);
    
};

struct NodeRps {
    
    std::vector<uint64_t> rps;
    
    std::string serialize() const;
    
    static NodeRps deserialize(const std::string &raw);
    
};

struct BalanceInfoWalletStats {
    
    size_t received = 0;
    size_t spent = 0;

    size_t firstReceived = 0;
    
    std::map<uint64_t, size_t> forgingSums;
    
    BalanceInfoWalletStats() = default;

    void plus(const TransactionInfo &tx, const Address &address, bool changeBalance, bool isForging);

};

struct NonForgingTxsBalance {
    size_t received = 0;
    size_t fees = 0;
    size_t count = 0;
    
    void plus(const TransactionInfo &tx);
};

struct NodeInfoForging {
    size_t receivedDelegate = 0;
    size_t spendDelegate = 0;
    
    size_t forged = 0;
    
    void plus(const TransactionInfo &tx, size_t valueUndelegate, bool isForging, bool isOkStatus) {
        if (isForging) {
            forged += tx.value;
            return;
        }
        if (!isOkStatus) {
            return;
        }
        if (tx.delegate.has_value()) {
            if (tx.delegate->isDelegate) {
                receivedDelegate += tx.delegate->value;
            } else {
                spendDelegate += valueUndelegate;
            }
        }
    }
};

struct NodesInfoForging {
    NodeInfoForging ourNode;
    NodeInfoForging enemyNode;
    
    void plus(const TransactionInfo &tx, size_t valueUndelegate, bool isOur, bool isForging, bool isOkStatus) {
        if (isOur) {
            ourNode.plus(tx, valueUndelegate, isForging, isOkStatus);
        } else {
            enemyNode.plus(tx, valueUndelegate, isForging, isOkStatus);
        }
    }
};

struct NodesInfoForgingForDays {
    std::map<uint64_t, NodesInfoForging> days;
    
    NodesInfoForging all;
    
    void plus(size_t day, const TransactionInfo &tx, size_t valueUndelegate, bool isOur, bool isForging, bool isOkStatus) {
        days[day].plus(tx, valueUndelegate, isOur, isForging, isOkStatus);
        all.plus(tx, valueUndelegate, isOur, isForging, isOkStatus);
    }
};

size_t getMaxBlockNumber(const std::vector<TransactionInfo> &infos);

size_t getMaxBlockNumber(const std::vector<TransactionStatus> &infos);

template<class Info>
bool isGreater(const Info &info, size_t blockNumber);

}

#endif // BLOCK_INFO_H_
