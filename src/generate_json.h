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

void genErrorResponse(int code, const std::string &error, rapidjson::Document &doc);

std::string genTransactionNotFoundResponse(const RequestId &requestId, const std::string &transaction, size_t countBlocks, size_t knownBlock);

void transactionToJson(const torrent_node_lib::TransactionInfo &info, const torrent_node_lib::BlockChainReadInterface &blockchain, size_t countBlocks, size_t knownBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

std::string transactionsToJson(const RequestId &requestId, const std::vector<torrent_node_lib::TransactionInfo> &infos, const torrent_node_lib::BlockChainReadInterface &blockchain, bool isFormat, const JsonVersion &version);

void addressesInfoToJson(const std::string &address, const std::vector<torrent_node_lib::TransactionInfo> &infos, const torrent_node_lib::BlockChainReadInterface &blockchain, size_t currentBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void balanceInfoToJson(const std::string &address, const torrent_node_lib::BalanceInfo &balance, size_t currentBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void blockHeaderToJson(const torrent_node_lib::BlockHeader &bh, const std::vector<torrent_node_lib::TransactionInfo> &signatures, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void blockHeadersToJson(const RequestId &requestId, const std::vector<torrent_node_lib::BlockHeader> &bh, const std::vector<std::vector<torrent_node_lib::TransactionInfo>> &signatures, uint64_t type, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void blockInfoToJson(const torrent_node_lib::BlockInfo &bi, const std::vector<torrent_node_lib::TransactionInfo> &signatures, uint64_t type, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void genCountBlockJson(size_t countBlocks, bool isFormat, const JsonVersion &version, rapidjson::Document &doc);

void genBlockDumpJson(const std::string &blockDump, bool isFormat, rapidjson::Document &doc);

std::string parseDumpBlockBinary(const std::string &response, bool isCompress);

std::vector<std::string> parseDumpBlocksBinary(const std::string &response, bool isCompress);

torrent_node_lib::MinimumBlockHeader parseBlockHeader(const std::string &response);

std::vector<torrent_node_lib::MinimumBlockHeader> parseBlocksHeader(const std::string &response);

#endif // GENERATE_JSON_H_
