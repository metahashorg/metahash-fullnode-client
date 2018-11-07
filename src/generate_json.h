#ifndef GENERATE_JSON_H_
#define GENERATE_JSON_H_

#include <string>
#include <variant>
#include <functional>

#include <rapidjson/document.h>

#include "BlockInfo.h"

namespace torrent_node_lib {
class BlockChainReadInterface;
}

struct RequestId {
    std::variant<std::string, size_t> id;
    bool isSet = false;
};

enum class JsonVersion {
    V1, V2
};

std::string genErrorResponse(const RequestId &requestId, int code, const std::string &error);

std::string genTransactionNotFoundResponse(const RequestId &requestId, const std::string &transaction, size_t countBlocks, size_t knwonBlock);

std::string transactionToJson(const RequestId &requestId, const torrent_node_lib::TransactionInfo &info, const torrent_node_lib::BlockChainReadInterface &blockchain, size_t countBlocks, size_t knwonBlock, bool isFormat, const JsonVersion &version);

std::string transactionsToJson(const RequestId &requestId, const std::vector<torrent_node_lib::TransactionInfo> &infos, const torrent_node_lib::BlockChainReadInterface &blockchain, bool isFormat, const JsonVersion &version);

void addressesInfoToJson(const std::string &address, const std::vector<torrent_node_lib::TransactionInfo> &infos, const torrent_node_lib::BlockChainReadInterface &blockchain, size_t currentBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void balanceInfoToJson(const std::string &address, const torrent_node_lib::BalanceInfo &balance, size_t currentBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

std::string blockHeaderToJson(const RequestId &requestId, const torrent_node_lib::BlockHeader &bh, const std::optional<std::reference_wrapper<const torrent_node_lib::BlockHeader>> &nextBlock, bool isFormat, const JsonVersion &version);

std::string blockHeadersToJson(const RequestId &requestId, const std::vector<torrent_node_lib::BlockHeader> &bh, bool isFormat, const JsonVersion &version);

std::string blockInfoToJson(const RequestId &requestId, const torrent_node_lib::BlockInfo &bi, const std::optional<std::reference_wrapper<const torrent_node_lib::BlockHeader>> &nextBlock, int type, bool isFormat, const JsonVersion &version);

std::string genCountBlockJson(const RequestId &requestId, size_t countBlocks, bool isFormat, const JsonVersion &version);

std::string genBlockDumpJson(const RequestId &requestId, const std::string &blockDump, bool isFormat);

#endif // GENERATE_JSON_H_
