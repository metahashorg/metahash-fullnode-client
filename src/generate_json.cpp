#include "generate_json.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "BlockChainReadInterface.h"

#include "jsonUtils.h"
#include "check.h"
#include "convertStrings.h"

using namespace common;
using namespace torrent_node_lib;

static void addIdToResponse(const RequestId &requestId, rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator) {
    if (requestId.isSet) {
        if (std::holds_alternative<std::string>(requestId.id)) {
            json.AddMember("id", strToJson(std::get<std::string>(requestId.id), allocator), allocator);
        } else {
            json.AddMember("id", std::get<size_t>(requestId.id), allocator);
        }
    }
}

template<typename Int>
static rapidjson::Value intOrString(Int intValue, bool isString, rapidjson::Document::AllocatorType &allocator) {
    if (isString) {
        return strToJson(std::to_string(intValue), allocator);
    } else {
        rapidjson::Value strVal;
        strVal.Set(intValue, allocator);
        return strVal;
    }
}

void genErrorResponse(int code, const std::string &error, rapidjson::Document &doc) {
    auto &allocator = doc.GetAllocator();
    
    rapidjson::Value errorJson(rapidjson::kObjectType);
    errorJson.AddMember("code", code, allocator);
    errorJson.AddMember("message", strToJson(error, allocator), allocator);
    doc.AddMember("error", errorJson, allocator);
}

std::string genTransactionNotFoundResponse(const RequestId& requestId, const std::string& transaction, size_t countBlocks, size_t knownBlock) {
    rapidjson::Document jsonDoc(rapidjson::kObjectType);
    auto &allocator = jsonDoc.GetAllocator();
    addIdToResponse(requestId, jsonDoc, allocator);
    
    rapidjson::Value errorJson(rapidjson::kObjectType);
    errorJson.AddMember("code", -32603, allocator);
    errorJson.AddMember("message", strToJson("Transaction " + transaction + " not found", allocator), allocator);

    rapidjson::Value errorData(rapidjson::kObjectType);
    errorData.AddMember("countBlocks", countBlocks, allocator);
    errorData.AddMember("knownBlock", knownBlock, allocator);
    errorJson.AddMember("data", errorData, allocator);

    jsonDoc.AddMember("error", errorJson, allocator);
    return jsonToString(jsonDoc, false);
}

static rapidjson::Value transactionInfoToJson(const TransactionInfo &info, const BlockHeader &bh, size_t currentBlock, rapidjson::Document::AllocatorType &allocator, int type, const JsonVersion &version) {
    CHECK(info.isInitialized, "Transaction not initialized");
    if (type == 2) {
        const bool isStringValue = version == JsonVersion::V2;
        
        rapidjson::Value infoJson(rapidjson::kObjectType);
        infoJson.AddMember("from", strToJson(info.fromAddress.calcHexString(), allocator), allocator);
        infoJson.AddMember("to", strToJson(info.toAddress.calcHexString(), allocator), allocator);
        infoJson.AddMember("value", intOrString(info.value, isStringValue, allocator), allocator);
        infoJson.AddMember("transaction", strToJson(toHex(info.hash.begin(), info.hash.end()), allocator), allocator);
        infoJson.AddMember("data", strToJson(toHex(info.data.begin(), info.data.end()), allocator), allocator);
        infoJson.AddMember("timestamp", intOrString(bh.timestamp, isStringValue, allocator), allocator);
        infoJson.AddMember("type", strToJson(bh.getBlockType(), allocator), allocator);
        if (info.blockNumber == 0) {
            infoJson.AddMember("blockNumber", intOrString(bh.blockNumber.value(), isStringValue, allocator), allocator);
        } else {
            infoJson.AddMember("blockNumber", intOrString(info.blockNumber, isStringValue, allocator), allocator);
        }
        infoJson.AddMember("signature", strToJson(toHex(info.sign.begin(), info.sign.end()), allocator), allocator);
        infoJson.AddMember("publickey", strToJson(toHex(info.pubKey.begin(), info.pubKey.end()), allocator), allocator);
        infoJson.AddMember("fee", intOrString(info.fees, isStringValue, allocator), allocator);
        infoJson.AddMember("nonce", intOrString(info.nonce, isStringValue, allocator), allocator);
        if (info.intStatus.has_value()) {
            infoJson.AddMember("intStatus", info.intStatus.value(), allocator);
        }
        if (info.delegate.has_value()) {
            if (info.delegate->isDelegate) {
                infoJson.AddMember("delegate", intOrString(info.delegate->value, isStringValue, allocator), allocator);
            } else {
                infoJson.AddMember("delegate", intOrString(0, isStringValue, allocator), allocator);
            }
            infoJson.AddMember("isDelegate", info.delegate->isDelegate, allocator);
        }
        if (!info.isStatusNeed()) {
            if (!info.intStatus.has_value() || !info.isIntStatusNotSuccess()) {
                infoJson.AddMember("status", strToJson("ok", allocator), allocator);
            } else {
                infoJson.AddMember("status", strToJson("error", allocator), allocator);
            }
        } else {
            if (!info.status.has_value()) {
                infoJson.AddMember("status", strToJson("pending", allocator), allocator);
            } else {
                infoJson.AddMember("status", strToJson(info.status->isSuccess ? "ok" : "error", allocator), allocator);
                if (std::holds_alternative<TransactionStatus::Delegate>(info.status->status)) {
                    // empty
                } else if (std::holds_alternative<TransactionStatus::UnDelegate>(info.status->status)) {
                    const TransactionStatus::UnDelegate &undelegateStatus = std::get<TransactionStatus::UnDelegate>(info.status->status);
                    infoJson.RemoveMember("delegate");
                    infoJson.AddMember("delegate", intOrString(undelegateStatus.value, isStringValue, allocator), allocator);
                    infoJson.AddMember("delegateHash", strToJson(toHex(undelegateStatus.delegateHash.begin(), undelegateStatus.delegateHash.end()), allocator), allocator);
                } else if (std::holds_alternative<TransactionStatus::V8Status>(info.status->status)) {
                    const TransactionStatus::V8Status &v8Status = std::get<TransactionStatus::V8Status>(info.status->status);
                    if (!info.status->isSuccess) {
                        if (v8Status.isServerError) {
                            infoJson.AddMember("isServerError", v8Status.isServerError, allocator);
                        }
                        if (v8Status.isScriptError) {
                            infoJson.AddMember("isScriptError", v8Status.isScriptError, allocator);
                        }
                    }
                    if (!v8Status.compiledContractAddress.isEmpty()) {
                        infoJson.AddMember("contractAddress", strToJson(v8Status.compiledContractAddress.calcHexString(), allocator), allocator);
                    }
                }
            }
        }
        return infoJson;
    } else if (type == 1) {
        return strToJson(toHex(info.hash.begin(), info.hash.end()), allocator);
    } else {
        throwUserErr("Incorrect transaction info type " + std::to_string(type));
    }
}

void transactionToJson(const TransactionInfo &info, const BlockChainReadInterface &blockchain, size_t countBlocks, size_t knownBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc) {
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resultValue(rapidjson::kObjectType);
    const BlockHeader &bh = blockchain.getBlock(info.blockNumber);
    resultValue.AddMember("transaction", transactionInfoToJson(info, bh, 0, allocator, 2, version), allocator);
    resultValue.AddMember("countBlocks", countBlocks, allocator);
    resultValue.AddMember("knownBlocks", knownBlock, allocator);
    doc.AddMember("result", resultValue, allocator);
}

void addressesInfoToJson(const std::string &address, const std::vector<TransactionInfo> &infos, const BlockChainReadInterface &blockchain, size_t currentBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc) {
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resultValue(rapidjson::kArrayType);
    for (const TransactionInfo &tx: infos) {
        const BlockHeader &bh = blockchain.getBlock(tx.blockNumber);
        resultValue.PushBack(transactionInfoToJson(tx, bh, currentBlock, allocator, 2, version), allocator);
    }
    doc.AddMember("result", resultValue, allocator);
}

std::string transactionsToJson(const RequestId &requestId, const std::vector<TransactionInfo> &infos, const BlockChainReadInterface &blockchain, bool isFormat, const JsonVersion &version) {
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    addIdToResponse(requestId, doc, allocator);
    rapidjson::Value resultValue(rapidjson::kArrayType);
    for (const TransactionInfo &tx: infos) {
        const BlockHeader &bh = blockchain.getBlock(tx.blockNumber);
        resultValue.PushBack(transactionInfoToJson(tx, bh, 0, allocator, 2, version), allocator);
    }
    doc.AddMember("result", resultValue, allocator);
    return jsonToString(doc, isFormat);
}

void balanceInfoToJson(const std::string &address, const BalanceInfo &balance, size_t currentBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc) {
    const bool isStringValue = version == JsonVersion::V2;
    
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resultValue(rapidjson::kObjectType);
    resultValue.AddMember("address", strToJson(address, allocator), allocator);
    resultValue.AddMember("received", intOrString(balance.received, isStringValue, allocator), allocator);
    resultValue.AddMember("spent", intOrString(balance.spent, isStringValue, allocator), allocator);
    resultValue.AddMember("count_received", intOrString(balance.countReceived, isStringValue, allocator), allocator);
    resultValue.AddMember("count_spent", intOrString(balance.countSpent, isStringValue, allocator), allocator);
    resultValue.AddMember("block_number", intOrString(balance.blockNumber, isStringValue, allocator), allocator);
    resultValue.AddMember("currentBlock", intOrString(currentBlock, isStringValue, allocator), allocator);
    if (balance.delegated.has_value()) {
        resultValue.AddMember("countDelegatedOps", intOrString(balance.delegated->countOp, isStringValue, allocator), allocator);
        resultValue.AddMember("delegate", intOrString(balance.delegated->delegate, isStringValue, allocator), allocator);
        resultValue.AddMember("undelegate", intOrString(balance.delegated->undelegate, isStringValue, allocator), allocator);
        resultValue.AddMember("delegated", intOrString(balance.delegated->delegated, isStringValue, allocator), allocator);
        resultValue.AddMember("undelegated", intOrString(balance.delegated->undelegated, isStringValue, allocator), allocator);
        resultValue.AddMember("reserved", intOrString(balance.delegated->reserved, isStringValue, allocator), allocator);
    }
    if (balance.forged.has_value()) {
        resultValue.AddMember("countForgedOps", intOrString(balance.forged->countOp, isStringValue, allocator), allocator);
        resultValue.AddMember("forged", intOrString(balance.forged->forged, isStringValue, allocator), allocator);
    }
    doc.AddMember("result", resultValue, allocator);
}

static rapidjson::Value blockHeaderToJson(const BlockHeader &bh, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, rapidjson::Document::AllocatorType &allocator, const JsonVersion &version) {
    const bool isStringValue = version == JsonVersion::V2;
    
    CHECK(bh.blockNumber.has_value(), "Block header not set");
    rapidjson::Value resultValue(rapidjson::kObjectType);
    resultValue.AddMember("type", strToJson(bh.getBlockType(), allocator), allocator);
    resultValue.AddMember("hash", strToJson(bh.hash, allocator), allocator);
    resultValue.AddMember("prev_hash", strToJson(bh.prevHash, allocator), allocator);
    resultValue.AddMember("tx_hash", strToJson(bh.txsHash, allocator), allocator);
    resultValue.AddMember("number", intOrString(bh.blockNumber.value(), isStringValue, allocator), allocator);
    resultValue.AddMember("timestamp", intOrString(bh.timestamp, isStringValue, allocator), allocator);
    CHECK(bh.countTxs.has_value(), "Count txs not set");
    resultValue.AddMember("count_txs", intOrString(bh.countTxs.value(), isStringValue, allocator), allocator);
    resultValue.AddMember("sign", strToJson(toHex(bh.signature), allocator), allocator);
    resultValue.AddMember("size", bh.blockSize, allocator);
    resultValue.AddMember("fileName", strToJson(bh.filePos.fileName, allocator), allocator);
        
    if (nextBlock.has_value()) {
        rapidjson::Value signaturesValue(rapidjson::kArrayType);
        for (const TransactionInfo &tx: nextBlock.value().get().blockSignatures) {
            const BlockHeader &bh = *nextBlock;
            signaturesValue.PushBack(transactionInfoToJson(tx, bh, 0, allocator, 2, version), allocator);
        }
        resultValue.AddMember("signatures", signaturesValue, allocator);
    }
    return resultValue;
}

void blockHeaderToJson(const BlockHeader &bh, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, bool isFormat, const JsonVersion &version, rapidjson::Document &doc) {
    if (bh.blockNumber == 0) {
        genErrorResponse(-32603, "Incorrect block number: 0. Genesis block begin with number 1", doc);
        return;
    }
    auto &allocator = doc.GetAllocator();
    doc.AddMember("result", blockHeaderToJson(bh, nextBlock, allocator, version), allocator);
}

std::string blockHeadersToJson(const RequestId &requestId, const std::vector<BlockHeader> &bh, bool isFormat, const JsonVersion &version) {
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    addIdToResponse(requestId, doc, allocator);
    rapidjson::Value vals(rapidjson::kArrayType);
    for (auto iter = bh.begin(); iter != bh.end();) {
        const BlockHeader &b  = *iter;
        const auto nextIter = ++iter;
        std::optional<std::reference_wrapper<const BlockHeader>> nextBlock;
        if (nextIter != bh.end()) {
            nextBlock = std::cref(*nextIter);
        }
        
        if (b.blockNumber == 0) {
            //genErrorResponse(requestId, -32603, "Incorrect block number: 0. Genesis block begin with number 1");
        }
        vals.PushBack(blockHeaderToJson(b, nextBlock, allocator, version), allocator);
        
        iter = nextIter;
    }
    doc.AddMember("result", vals, allocator);
    return jsonToString(doc, isFormat);
}

void blockInfoToJson(const BlockInfo &bi, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, int type, bool isFormat, const JsonVersion &version, rapidjson::Document &doc) {
    const BlockHeader &bh = bi.header;
    
    if (bh.blockNumber == 0) {
        genErrorResponse(-32603, "Incorrect block number: 0. Genesis block begin with number 1", doc);
        return;
    }
    
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resultValue = blockHeaderToJson(bh, nextBlock, allocator, version);
    rapidjson::Value txs(rapidjson::kArrayType);
    for (const TransactionInfo &tx: bi.txs) {
        txs.PushBack(transactionInfoToJson(tx, bi.header, 0, allocator, type, version), allocator);
    }
    resultValue.AddMember("txs", txs, allocator);
    doc.AddMember("result", resultValue, allocator);
}

void genCountBlockJson(size_t countBlocks, bool isFormat, const JsonVersion &version, rapidjson::Document &doc) {
    const bool isStringValue = version == JsonVersion::V2;
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resultValue(rapidjson::kObjectType);
    resultValue.AddMember("count_blocks", intOrString(countBlocks, isStringValue, allocator), allocator);
    doc.AddMember("result", resultValue, allocator);
}

void genBlockDumpJson(const std::string &blockDump, bool isFormat, rapidjson::Document &doc) {
    auto &allocator = doc.GetAllocator();
    rapidjson::Value resultValue(rapidjson::kObjectType);
    resultValue.AddMember("dump", strToJson(blockDump, allocator), allocator);
    doc.AddMember("result", resultValue, allocator);
}
