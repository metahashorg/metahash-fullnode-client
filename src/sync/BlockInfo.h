#ifndef BLOCK_INFO_H_
#define BLOCK_INFO_H_

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <set>
#include <unordered_map>

#include "duration.h"

#include "Address.h"

namespace torrent_node_lib {

struct FilePosition {
    size_t pos;
    std::string fileNameRelative;
    
    FilePosition() = default;
    
    FilePosition(const std::string &fileNameRelative, size_t pos)
        : pos(pos)
        , fileNameRelative(fileNameRelative)
    {}
    
    std::string serialize() const;
    
    void serialize(std::vector<char> &buffer) const;
    
    static FilePosition deserialize(const std::string &raw, size_t from, size_t &nextFrom);
    
    static FilePosition deserialize(const std::string &raw, size_t &from);
    
    static FilePosition deserialize(const std::string &raw);
    
};

struct AddressInfo {
    
    AddressInfo() = default;
    
    AddressInfo(size_t pos, const std::string &fileName, size_t blockNumber, size_t index)
        : filePos(fileName, pos)
        , blockNumber(blockNumber)
        , blockIndex(index)
    {}
    
    FilePosition filePos;
    
    size_t blockNumber = 0;
    size_t blockIndex = 0;
    
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
        enum class ScriptType {
            compile, run, pay, unknown
        };
        
        std::string txRaw;
        ScriptType type = ScriptType::unknown;
    };
    
    std::string hash;
    Address fromAddress;
    Address toAddress;
    int64_t value;
    int64_t fees = 0;
    uint64_t nonce = 0;
    size_t blockNumber = 0;
    size_t blockIndex = 0;
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
    size_t countTxs = 0;
    
    std::optional<DelegateBalance> delegated;
    
    std::optional<ForgedBalance> forged;
    
    std::optional<size_t> hash;
    
    size_t blockNumber = 0;
    
    BalanceInfo() = default;
    
    void plusWithDelegate(const TransactionInfo &tx, const Address &address, const std::optional<int64_t> &undelegateValue, bool isOkStatus);
    
    void plusWithoutDelegate(const TransactionInfo &tx, const Address &address, bool changeBalance, bool isForging);
        
    BalanceInfo& operator+=(const BalanceInfo &second);
    
    int64_t calcBalance();
    
    int64_t calcBalanceWithoutDelegate();
    
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
    uint64_t blockSize = 0;
    uint64_t blockType = 0;
    std::vector<unsigned char> hash;
    std::vector<unsigned char> prevHash;
    std::vector<unsigned char> txsHash;
        
    std::vector<unsigned char> signature;
       
    size_t countTxs = 0;
    
    FilePosition filePos;
        
    std::optional<size_t> blockNumber;
        
    std::vector<unsigned char> senderSign;
    std::vector<unsigned char> senderPubkey;
    std::vector<unsigned char> senderAddress;
    
    std::string serialize() const;
    
    static BlockHeader deserialize(const std::string &raw);
    
    bool isStateBlock() const;
    
    bool isSimpleBlock() const;
    
    bool isForgingBlock() const;
    
    std::string getBlockType() const;
    
    size_t endBlockPos() const;
};

struct MinimumBlockHeader {
    size_t number;
    size_t blockSize;
    std::string hash;
    std::string parentHash;
    std::string fileName;
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
    
    std::vector<TransactionInfo> getBlockSignatures() const;
};

struct BlocksMetadata {
    std::vector<unsigned char> blockHash;
    std::vector<unsigned char> prevBlockHash;
       
    BlocksMetadata() = default;
    
    BlocksMetadata(const std::vector<unsigned char> &blockHash, const std::vector<unsigned char> &prevBlockHash)
        : blockHash(blockHash)
        , prevBlockHash(prevBlockHash)
    {}
    
    std::string serialize() const;
       
    static BlocksMetadata deserialize(const std::string &raw);
    
};

struct MainBlockInfo {
    size_t blockNumber = 0;
    std::vector<unsigned char> blockHash;
    size_t countVal = 0;
    
    MainBlockInfo() = default;
    
    MainBlockInfo(size_t blockNumber, const std::vector<unsigned char> &blockHash, size_t countVal)
        : blockNumber(blockNumber)
        , blockHash(blockHash)
        , countVal(countVal)
    {}
    
    std::string serialize() const;
    
    static MainBlockInfo deserialize(const std::string &raw);
    
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

struct ForgingSums {
    std::unordered_map<uint64_t, size_t> sums;
    size_t blockNumber = 0;
    
    std::string serialize() const;
    
    static ForgingSums deserialize(const std::string &raw);
    
    ForgingSums& operator +=(const ForgingSums &second);
    
};

size_t getMaxBlockNumber(const std::vector<TransactionInfo> &infos);

size_t getMaxBlockNumber(const std::vector<TransactionStatus> &infos);

template<class Info>
bool isGreater(const Info &info, size_t blockNumber);

}

#endif // BLOCK_INFO_H_
