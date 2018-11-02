#ifndef GENERATE_JSON_H_
#define GENERATE_JSON_H_

#include <string>
#include <variant>
#include <functional>

#include "BlockInfo.h"

class BlockChainReadInterface;

struct RequestId {
    std::variant<std::string, size_t> id;
    bool isSet = false;
};

enum class JsonVersion {
    V1, V2
};

std::string genErrorResponse(const RequestId &requestId, int code, const std::string &error);

std::string genTransactionNotFoundResponse(const RequestId &requestId, const std::string &transaction, size_t countBlocks, size_t knwonBlock);

std::string transactionToJson(const RequestId &requestId, const TransactionInfo &info, const BlockChainReadInterface &blockchain, size_t countBlocks, size_t knwonBlock, bool isFormat, const JsonVersion &version);

std::string transactionsToJson(const RequestId &requestId, const std::vector<TransactionInfo> &infos, const BlockChainReadInterface &blockchain, bool isFormat, const JsonVersion &version);

std::string addressesInfoToJson(const RequestId &requestId, const std::string &address, const std::vector<TransactionInfo> &infos, const BlockChainReadInterface &blockchain, size_t currentBlock, bool isFormat, const JsonVersion &version);

std::string balanceInfoToJson(const RequestId &requestId, const std::string &address, const BalanceInfo &balance, size_t currentBlock, bool isFormat, const JsonVersion &version);

std::string blockHeaderToJson(const RequestId &requestId, const BlockHeader &bh, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, bool isFormat, const JsonVersion &version);

std::string blockHeadersToJson(const RequestId &requestId, const std::vector<BlockHeader> &bh, bool isFormat, const JsonVersion &version);

std::string blockInfoToJson(const RequestId &requestId, const BlockInfo &bi, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, int type, bool isFormat, const JsonVersion &version);

std::string genCountBlockJson(const RequestId &requestId, size_t countBlocks, bool isFormat, const JsonVersion &version);

std::string genBlockDumpJson(const RequestId &requestId, const std::string &blockDump, bool isFormat);

#endif // GENERATE_JSON_H_
