#include "StatisticsServer.h"

#include <fstream>

#include "check.h"
#include "log.h"
#include "stopProgram.h"
#include "common/curlWrapper.h"
#include "stringUtils.h"
#include "network_utils.h"
#include "benchmarks.h"
#include "convertStrings.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "jsonUtils.h"

using namespace common;

namespace torrent_node_lib {

static rapidjson::Document request(const std::string &url, const std::string &postData) {
    const std::string buffer = Curl::request(url, postData, "", "");
    
    rapidjson::Document doc;
    const rapidjson::ParseResult pr = doc.Parse(buffer.c_str());
    CHECK(pr, "rapidjson parse error. Data: " + buffer + ". key " + postData);
    
    CHECK(!doc.HasMember("error") || doc["error"].IsNull(), jsonToString(doc["error"], false) + ". Key " + postData);
    
    return doc;
}

StatisticsServer::StatisticsServer(const std::string& serverStatName, const std::string &network, const std::string &group, const std::string &server, const std::string &latencyFile, const std::string &version) 
    : serverStatName(serverStatName)
    , network(network)
    , group(group)
    , server(server)
    , latencyFile(latencyFile)
    , version(version)
{
    Curl::initialize();
    thisIp = getMyIp2(server);
}

void StatisticsServer::start() {
    workThread = std::thread(&StatisticsServer::processThread, this);
}

void StatisticsServer::join() {
    if (workThread.joinable()) {
        workThread.join();
    }
}

StatisticsServer::~StatisticsServer() {
    if (workThread.joinable()) {
        LOGERR << "Statistics thread not stopped";
        workThread.detach();
    }
}

size_t StatisticsServer::getLatency() {
    std::ifstream file(latencyFile.c_str());
    if (!file.good()) {
        return 0;
    }
    size_t result;
    file >> result;
    return result;
}

void StatisticsServer::abort() {
    aborted = true;
}

void StatisticsServer::processThread() {
    try {
        while (!aborted) {
            Timer tt;
            try {                
                std::unique_lock<std::mutex> lock(blockMutex);
                const BlockStatistic lastBlockCopy = lastBlock;
                lock.unlock();
                std::unique_lock<std::mutex> lock2(requestStatMutex);
                const RequestsStat requestStatCopy = requestStat;
                requestStat.clear();
                lock2.unlock();

                rapidjson::Document doc(rapidjson::kObjectType);
                auto &allocator = doc.GetAllocator();
                rapidjson::Value paramsJson(rapidjson::kObjectType);
                
                const time_point_system now = ::nowSystem();
                paramsJson.AddMember("network", strToJson(network, allocator), allocator);
                paramsJson.AddMember("group", strToJson(group, allocator), allocator);
                paramsJson.AddMember("server", strToJson(serverStatName, allocator), allocator);
                paramsJson.AddMember("timestamp_ms", getTimestampMs(now), allocator);
                
                rapidjson::Value metricsJson(rapidjson::kArrayType);
                
                auto addParamFunc = [this, &metricsJson, &allocator](const std::string &type, const std::string &metric, auto value) {
                    rapidjson::Value paramJson(rapidjson::kObjectType);
                    
                    paramJson.AddMember("type", strToJson(type, allocator), allocator);
                    paramJson.AddMember("metric", strToJson(metric, allocator), allocator);
                    paramJson.AddMember("value", value, allocator);
                    
                    metricsJson.PushBack(paramJson, allocator);
                };
                
                auto addBlockStatFunc = [this, &addParamFunc, &allocator](const BlockStatistic &stat, const std::string &prefix) {
                    addParamFunc("none", prefix + "time_work", (stat.bt.timeGet + stat.bt.timeSave).count());
                    addParamFunc("none", prefix + "latency", stat.bt.timeAll.count());
                    std::string blockInfo;
                    blockInfo += "number: " + std::to_string(stat.bh.blockNumber.value()) + "; ";
                    blockInfo += "type: " + std::to_string(stat.bh.blockType) + "; ";
                    blockInfo += "size: " + std::to_string(stat.bh.blockSize) + "; ";
                    blockInfo += "hash: " + toHex(stat.bh.hash) + "; ";
                    blockInfo += "prev_hash: " + toHex(stat.bh.prevHash) + "";
                    addParamFunc("none", prefix + "block_info", strToJson(blockInfo, allocator));
                    addParamFunc("none", prefix + "transactions_init", stat.txSs.countInitTxs);
                    addParamFunc("none", prefix + "transactions_transfer", stat.txSs.countTransferTxs);
                };
                
                if (lastBlockCopy.tp.has_value()) {
                    addBlockStatFunc(lastBlockCopy, "last_");
                }
                
                addParamFunc("sum", "count_requests", requestStatCopy.allRequests);
                
                for (const auto &[key, value]: requestStatCopy.requests) {
                    addParamFunc("sum", "count_requests_" + key, value);
                }
                
                const size_t latency = getLatency();
                addParamFunc("none", "core_latency", latency);
                
                addParamFunc("none", "ip", strToJson(thisIp, allocator));
                
                addParamFunc("none", "version", strToJson(version, allocator));
                
                const BenchmarkInfo benchmarks = getBench();
                addParamFunc("none", "bench_mem", benchmarks.memory);
                addParamFunc("none", "bench_io", benchmarks.ioTest);
                addParamFunc("none", "bench_io2", benchmarks.ioTest2);
                addParamFunc("none", "bench_ssl", benchmarks.opensslTest);
                
                paramsJson.AddMember("metrics", metricsJson, allocator);
                doc.AddMember("params", paramsJson, allocator);
                
                std::string url = server;
                if (url[url.size() - 1] != '/') {
                    url += '/';
                }
                url += "save-metrics";
                const rapidjson::Document response = request(url, jsonToString(doc, false));
                CHECK(response.HasMember("result") && response["result"].IsString() && response["result"].GetString() == std::string("ok"), "response field not found " + jsonToString(response, false));               
            } catch (const exception &e) {
                LOGERR << e;
            } catch (const std::exception &e) {
                LOGERR << e.what();
            }
            
            tt.stop();
            const milliseconds workedMs(tt.countMs());
            std::this_thread::sleep_for(1s - workedMs);
            
            checkStopSignal();
        }
    } catch (const StopException &e) {
        LOGINFO << "Stop Statistics::processThread";
        return;
    }
}

StatisticsServer::BlockStatistic StatisticsServer::makeBlockStatistic(const BlockInfo& bi) {
    BlockStatistic blockStat;
       
    blockStat.bh = bi.header;
    
    blockStat.tp = ::nowSystem();
    
    return blockStat;
}

void StatisticsServer::saveNewBlock(const BlockInfo &bi) {
    const BlockStatistic blockStat = makeBlockStatistic(bi);
    
    std::lock_guard<std::mutex> lock(blockMutex);
    lastBlock = blockStat;
}

void StatisticsServer::setLastBlock(const BlockHeader& bh) {
    BlockStatistic blockStat;
    blockStat.bh = bh;
    blockStat.tp = ::nowSystem();
    
    std::lock_guard<std::mutex> lock(blockMutex);
    lastBlock = blockStat;   
}

void StatisticsServer::incRequest(const std::string &request) {
    std::lock_guard<std::mutex> lock(requestStatMutex);
    requestStat.incRequest(request);
}

}
