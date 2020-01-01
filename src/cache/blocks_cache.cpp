#include "blocks_cache.h"
#include <fstream>
#include <byteswap.h>
#include <boost/exception/all.hpp>
#include <boost/asio/io_context.hpp>
#include "common/stopProgram.h"
#include "common/filesystem_utils.h"
#include "common/string_utils.h"
#include "settings/settings.h"
#include "log.h"
#include "json_rpc.h"
#include "http_json_rpc_request.h"
#include "../sync/BlockInfo.h"
#include "../sync/synchronize_blockchain.h"
#include "compress.h"
#include "convertStrings.h"
#include "task_handlers/get_count_blocks_handler.h"
#include "task_handlers/get_dump_block_by_number_handler.h"
#include "blocks_exceptions.h"

#define CACHE_BGN try

#define CACHE_END(ret) \
    catch (const common::StopException&) {\
        LOGINFO << __PRETTY_FUNCTION__ << " Stop invoke";\
        ret;\
    } catch (const std::bad_variant_access& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " bad variant exception: " << ex.what();\
        ret;\
    } catch (const boost::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);\
        ret;\
    } catch (const std::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();\
        ret;\
    } catch (...) {\
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";\
        ret;\
    }

//const blocks_cache::blk_number blocks_cache::extra_blocks_epoch = 1835000;

blocks_cache::blocks_cache()
    : m_run(false)
    , m_nextblock(0)
    , m_last_signed_block(0)
{
}

bool blocks_cache::init()
{
    CACHE_BGN
    {
        static bool cache_init = false;

        if (cache_init) {
            return true;
        }
        if (!fs_utils::dir::is_exists("./data/")){
            if (!fs_utils::dir::create("./data/")) {
                LOGERR << "Cache. Could not create folder ./data/";
                return false;
            };
        }
        if (!fs_utils::dir::is_exists("./data/cache/")){
            if (!fs_utils::dir::create("./data/cache/")) {
                LOGERR << "Cache. Could not create folder ./data/cache/";
                return false;
            };
        }

        m_dbcache.reset(leveldb::NewLRUCache(500*1024*1024));
        m_dbpolicy.reset(leveldb::NewBloomFilterPolicy(10));

        leveldb::Options options;
        options.create_if_missing = true;
        options.block_cache = m_dbcache.get();
        options.filter_policy = m_dbpolicy.get();

        leveldb::DB* db = nullptr;
        leveldb::Status status = leveldb::DB::Open(options, "./data/cache/", &db);
        if (!status.ok()) {
            LOGERR << "Cache. Could not open database. " << status.ToString().c_str();
            return false;
        }
        m_db.reset(db);

        std::string result;
        leveldb::ReadOptions opt;
        status = m_db->Get(opt, "next_block", &result);
        if (status.ok()) {
            m_nextblock = static_cast<blk_number>(std::atoi(result.c_str()));
        }
        result.clear();
        status = m_db->Get(opt, "last_signed_block", &result);
        if (status.ok()) {
            m_last_signed_block = static_cast<blk_number>(std::atoi(result.c_str()));
        }
        cache_init = true;

        LOGINFO << "Cache. Init successfully";

        return cache_init;
    }
    CACHE_END(return false)
}

bool blocks_cache::start()
{
    CACHE_BGN
    {
        if (!init()) {
            LOGERR << "Cache. Could not initialize. Aborted.";
        }
        LOGINFO << "Cache. Next block is " << m_nextblock;

        m_worker = std::make_unique<std::thread>(worker_proc, this);
        return true;
    }
    CACHE_END(return false)
}

void blocks_cache::stop()
{
    CACHE_BGN
    {
        m_run = false;
        if (m_worker.get() && m_worker->joinable()) {
            m_worker->join();
        }
    }
    CACHE_END()
}

bool blocks_cache::runing() const
{
    return m_run;
}

blocks_cache::blk_number blocks_cache::next_block() const
{
    return m_nextblock;
}

blocks_cache::blk_number blocks_cache::last_signed_block() const
{
    return m_last_signed_block;
}

void blocks_cache::worker_proc(blocks_cache *param)
{
    CACHE_BGN
    {
        param->m_run = true;
        switch (settings::system::blocks_cache_ver) {
        case 2:
            param->routine_2();
            break;
        default:
            param->routine();
            break;
        }
    }
    CACHE_END()
    param->m_run = false;
    LOGINFO << "Cache. Stoped";
}

void blocks_cache::routine()
{
    CACHE_BGN
    {
        LOGINFO << "Cache. Version 1 Started";
        blk_number count_blocks = 0;
        std::string json;
        json_rpc_reader reader;
        const rapidjson::Value* tmp;
        std::chrono::system_clock::time_point tp;

        const json_response_type* response;

        http_json_rpc_request_ptr gcb = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        gcb->set_path("get-count-blocks");
        gcb->set_body("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-count-blocks\"}");

        http_json_rpc_request_ptr gdbn = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        gdbn->set_path("get-dump-block-by-number");

        ext_blk_data ext_data = {0}; // does not check signs

        while (m_run) {
            common::checkStopSignal();
            if (std::chrono::high_resolution_clock::now() < tp) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(30);

            gcb->set_host(settings::server::get_tor());
            gcb->reset_attempts();
            gcb->execute();
            response = gcb->get_response();
            if (!response) {
                LOGERR << "Cache. Could not get response from get-count-blocks";
                continue;
            }
            if (response->get().body().empty()) {
                LOGERR << "Cache. Could not get get-count-blocks";
                continue;
            }
            if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                LOGERR << "Cache. Could not parse get-count-blocks (" << reader.get_parse_error() << "): " << reader.get_parse_error_str();
                continue;
            }
            tmp = reader.get_result();
            if (tmp == nullptr) {
                LOGERR << "Cache. Did not find result in get-count-blocks";
                continue;
            }
            if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                LOGERR << "Cache. Did not find field 'count_blocks' in get-count-blocks";
                continue;
            }
            if (m_nextblock == 0) {
                m_nextblock = count_blocks - settings::system::blocks_cache_init_count > 0 ? count_blocks - settings::system::blocks_cache_init_count : 1;
            }

            while (m_run && m_nextblock <= count_blocks) {
                json.clear();
                string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dump-block-by-number\", \"params\":{\"number\":", std::to_string(m_nextblock), ", \"compress\":true}}");
                gdbn->set_body(json);
                gdbn->set_host(settings::server::get_tor());
                gdbn->reset_attempts();
                gdbn->execute();
                response = gdbn->get_response();
                if (response->get().body().empty()) {
                    LOGERR << "Cache. Could not get get-dump-block-by-number";
                    break;
                }
                if (reader.parse(response->get().body().c_str(), response->get().body().size())) {
                    tmp = reader.get_error();
                    if (tmp) {
                        LOGERR << "Cache. get-count-blocks error: " << reader.stringify(tmp);
                    } else {
                        LOGERR << "Cache. get-count-blocks error: json response occured " << response->get().body().size() << " bytes: " << response->get().body();
                    }
                    break;
                }

                std::string buf = torrent_node_lib::decompress(response->get().body());
                auto bi = torrent_node_lib::Sync::parseBlockDump(buf, false);
                if (!std::holds_alternative<torrent_node_lib::BlockInfo>(bi)) {
                    LOGERR << "Cache. Block could not parsed";
                } else {
                    torrent_node_lib::BlockInfo &b = std::get<torrent_node_lib::BlockInfo>(bi);
                    if (save_block(m_nextblock,
                                   string_utils::bin2hex(b.header.hash),
                                   response->get().body(),
                                   std::string_view(ext_data, sizeof(ext_data)))) {
                        update_number(++m_nextblock);
                    }
                }
                common::checkStopSignal();
            }
        }
    }
    CACHE_END()
}

void blocks_cache::routine_2()
{
    CACHE_BGN
    {
        LOGINFO << "Cache. Version 2 Started";
        std::string json;
        json.resize(32768);
        std::string decompressed;
        json_rpc_reader reader;

        const rapidjson::Value* tmp = nullptr;
        rapidjson::Value::ConstMemberIterator it;
        rapidjson::Value::Array::ConstValueIterator arr_it, hash_it;

        std::chrono::system_clock::time_point tp;

        std::vector<blocks_cache::blk_info> hashes;
        std::vector<blocks_cache::blk_info>::iterator iter;
        const char
                *buf = nullptr,
                *p = nullptr;
        uint64_t blk_size = 0;
        size_t resp_size = 0;

        const json_response_type* response;

        torrent_node_lib::BlockInfo bi, bi_prev;
        std::string tmp_str;
        tmp_str.resize(32768);

        ext_blk_data ext_data = {0};

        http_json_rpc_request_ptr get_count = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        get_count->set_path("get-count-blocks");
        get_count->set_body("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-count-blocks\", \"params\":{ \"type\": \"forP2P\"}}");

        blk_number tmp_blk_num = 0;
        if (m_nextblock == 0) {
            m_nextblock = 1;
            for (;;) {
                if (settings::system::blocks_cache_init_count == 0) {
                    break;
                }
                get_count->execute();
                response = get_count->get_response();
                if (!response) {
                    LOGERR << "Cache. Could not get response from get-count-blocks";
                    break;
                }
                if (response->get().body().empty()) {
                    LOGERR << "Cache. Could not get get-count-blocks";
                    break;
                }
                if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                    LOGERR << "Cache. Could not parse get-count-blocks (" << reader.get_parse_error() << "): " << reader.get_parse_error_str();
                    break;
                }
                tmp = reader.get_result();
                if (tmp == nullptr) {
                    LOGERR << "Cache. Did not find result in et-count-blocks";
                    break;
                }
                blk_number count_blocks = 0;
                if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                    LOGERR << "Cache. Did not find field 'count_blocks' in get-count-blocks";
                    break;
                }
                m_nextblock = count_blocks > settings::system::blocks_cache_init_count ? count_blocks - settings::system::blocks_cache_init_count : count_blocks;
                break;
            }
        }

        LOGINFO << "Cache. Next block set to " << m_nextblock;

        // max summary size of blocks per request
        // increase approximately up to 2 times because of compress
        const unsigned int blocks_max_size = settings::system::blocks_cache_recv_data_size * 1024 * 1024 * 2;
        unsigned int blocks_size = 0;

        http_json_rpc_request_ptr get_blocks = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        get_blocks->set_path("get-blocks");

        http_json_rpc_request_ptr get_dumps = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        get_dumps->set_path("get-dumps-blocks-by-hash");

        http_json_rpc_request_ptr get_block = std::make_shared<http_json_rpc_request>(settings::server::get_tor());
        get_block->set_path("get-block-by-number");

        auto load_extra_blocks = [&]() {
            while (!hashes.empty()) {
                iter = hashes.begin();
                size_t count = 0;
                json.clear();
                json.append("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dumps-blocks-by-hash\", \"params\":{\"compress\":true, \"hashes\":[");
                for (; iter!= hashes.end(); iter++) {
                    for (auto extra_it = iter->extra.cbegin(); extra_it != iter->extra.cend(); extra_it++, count++) {
                        if (iter != hashes.begin()) {
                            json.append(",");
                        }
                        json.append("\"");
                        json.append(string_utils::bin2hex(*extra_it));
                        json.append("\"");
                    }
                    if (settings::system::blocks_cache_recv_count <= count) {
                        break;
                    }
                }
                json.append("]}}");

                if (count == 0) {
                    break;
                }

                get_dumps->set_host(settings::server::get_tor());
                get_dumps->set_body(json);
                get_dumps->reset_attempts();
                get_dumps->execute();
                response = get_dumps->get_response();
                if (response && !response->get().body().empty()) {
                    if (reader.parse(response->get().body().c_str(), response->get().body().size())) {
                        tmp = reader.get_error();
                        if (tmp) {
                            LOGERR << "Cache. Extra blocks. get-dumps-blocks-by-hash got error: " << reader.stringify(tmp);
                        } else {
                            LOGERR << "Cache. Extra blocks. get-dumps-blocks-by-hash body has parsed " << response->get().body().size() << " bytes";
                        }
                    } else {
                        decompressed = torrent_node_lib::decompress(response->get().body());
                        resp_size = decompressed.size();

                        LOGINFO << "Cache. Extra blocks. Getting " << count << " block(s). Size compressed " << response->get().body().size() << "/ decompressed " << resp_size;

                        buf = decompressed.c_str();
                        p = buf;
                        auto it = hashes.cbegin();
                        while (p < buf + resp_size && it != hashes.cend()) {
                            memcpy(&blk_size, p, sizeof(blk_size));
                            blk_size = __builtin_bswap64(blk_size);
                            if (blk_size == 0) {
                                LOGERR << "Cache. Extra blocks. Error while response parse";
                                break;
                            }
                            p += sizeof(blk_size);
                            memset(ext_data, 0, sizeof(ext_data));

                            auto some_block = torrent_node_lib::Sync::parseBlockDump(std::string(p, blk_size), false);

                            if (!std::holds_alternative<torrent_node_lib::SignBlockInfo>(some_block)) {
                                LOGERR << "Cache. Extra blocks. Extra Block in " << it->number << " is not Sign Block";
                            } else {
                                torrent_node_lib::SignBlockInfo& sign_block = std::get<torrent_node_lib::SignBlockInfo>(some_block);
                                if (core_addr_verification(sign_block) > 0) {
                                    ext_data[0] = blk_signed;
                                }
                                tmp_str.clear();
                                if (get_block_num_by_hash(string_utils::bin2hex(sign_block.header.prevHash), tmp_str)) {
                                    save_extra_data(tmp_str, std::string_view(ext_data, sizeof(ext_data)));
                                    save_extra_block(tmp_str, std::string_view(p, blk_size));
                                    blk_number number = static_cast<blk_number>(std::atoi(tmp_str.c_str()));
                                    if (ext_data[0] == blk_signed && number > m_last_signed_block) {
                                        m_last_signed_block = number;
                                        update_last_signed(m_last_signed_block);
                                    }
                                } else {
                                    // TODO
                                    // query from torrent ? most likely block should be in the cache
                                    LOGERR << "Cache. Extra blocks. Could not find block #" << tmp_str;
                                }
                            }
                            p += blk_size;
                            it++;
                        }
                    }
                } else {
                    LOGERR << "Cache. Extra blocks. Could not get get-dumps-blocks-by-hash response";
                }
                hashes.erase(hashes.begin(), iter);
            }
        };

        bool force_download = settings::system::blocks_cache_force;

        while (m_run) {
            common::checkStopSignal();            

            if (!force_download && std::chrono::high_resolution_clock::now() < tp) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(15);

            hashes.clear();

            json.clear();
            string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-blocks\", \"params\":{\"beginBlock\":",
                std::to_string(m_nextblock), ", \"countBlocks\": ", std::to_string(settings::system::blocks_cache_recv_count),
                ", \"type\": \"forP2P\", \"direction\": \"forward\"}}");

            get_blocks->set_host(settings::server::get_tor());
            get_blocks->set_body(json);
            get_blocks->reset_attempts();
            get_blocks->execute();
            response = get_blocks->get_response();
            if (!response) {
                LOGERR << "Cache. Could not get response from get-blocks";
                continue;
            }
            if (response->get().body().empty()) {
                LOGERR << "Cache. Could not get get-blocks";
                continue;
            }
            if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                LOGERR << "Cache. Could not parse get-blocks (" << reader.get_parse_error() << "): " << reader.get_parse_error_str();
                continue;
            }
            tmp = reader.get_error();
            if (tmp != nullptr) {
                LOGERR << "Cache. Got error from get-blocks: " << reader.stringify(tmp);
                continue;
            }
            tmp = reader.get_result();
            if (tmp == nullptr) {
                LOGERR << "Cache. Did not find result in get-blocks";
                continue;
            }
            if (!tmp->IsArray()) {
                LOGERR << "Cache. Expect array from get-blocks";
                continue;
            }
            if (tmp->Size() == 0) {
                get_count->reset_attempts();
                get_count->execute();
                response = get_count->get_response();
                if (response->get().body().empty()) {
                    LOGERR << "Cache. Could not get get-count-blocks";
                    continue;
                }
                if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                    LOGERR << "Cache. Could not parse get-count-blocks (" << reader.get_parse_error() << "): " << reader.get_parse_error_str();
                    continue;
                }
                tmp = reader.get_result();
                if (tmp == nullptr) {
                    LOGERR << "Cache. Did not find result in get-count-blocks";
                    continue;
                }
                blk_number count_blocks = 0;
                if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                    LOGERR << "Cache. Did not find field 'count_blocks' in get-count-blocks";
                    continue;
                }

                force_download = false;

                if (count_blocks == m_last_signed_block) {
                    continue;
                }
                hashes.emplace_back(blk_info(count_blocks, {}));
                it = tmp->FindMember("next_extra_blocks");
                if (it != tmp->MemberEnd() && it->value.IsArray()) {
                    for (hash_it = it->value.GetArray().begin(); hash_it != it->value.GetArray().end(); hash_it++) {
                        hashes.back().extra.emplace_back(common::fromHex({hash_it->GetString(), hash_it->GetStringLength()}));
                    }
                }
                load_extra_blocks();
                continue;
            }

            force_download = settings::system::blocks_cache_force;
            hashes.clear();
            blocks_size = 0;

            for (arr_it = tmp->GetArray().begin(); arr_it != tmp->GetArray().end(); arr_it++) {
                if (!arr_it->IsObject()) {
                    continue;
                }
                it = arr_it->FindMember("size");
                if (it != arr_it->MemberEnd()) {
                    blocks_size += it->value.GetUint();
                    if (settings::system::blocks_cache_block_verification ? hashes.size() > 1 : hashes.size() > 0) {
                        if (blocks_size > blocks_max_size) {
                            LOGWARN << "Cache. Reached the maximum of the blocks data size per one request (" << blocks_max_size << " bytes): " << hashes.size() << " blocks / " << blocks_size << " bytes";
                            break;
                        }
                    }
                }
                it = arr_it->FindMember("number");
                if (it == arr_it->MemberEnd()) {
                    LOGERR << "Cache. Could not get block number from get-blocks";
                    hashes.clear();
                    break;
                }
                tmp_blk_num = it->value.GetUint();
                it = arr_it->FindMember("hash");
                if (it == arr_it->MemberEnd()) {
                    LOGERR << "Cache. Could not get block hash from get-blocks";
                    hashes.clear();
                    break;
                }

                hashes.emplace_back(tmp_blk_num, common::fromHex({it->value.GetString(), it->value.GetStringLength()}));

                it = arr_it->FindMember("prev_extra_blocks");
                if (it != arr_it->MemberEnd() && it->value.IsArray() && it->value.Size() > 0) {
                    for (hash_it = it->value.GetArray().begin(); hash_it != it->value.GetArray().end(); hash_it++) {
                        hashes.back().extra.emplace_back(common::fromHex({hash_it->GetString(), hash_it->GetStringLength()}));
                    }
                }
            }

            if (hashes.empty()/* || (settings::system::blocks_cache_block_verification && m_nextblock < extra_blocks_epoch && hashes.size() < 2)*/) {
                continue;
            }

            json.clear();
            json.append("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dumps-blocks-by-hash\", \"params\":{\"compress\":true, \"hashes\":[");
            for (auto i = hashes.cbegin(); i != hashes.cend(); i++) {
                if (i != hashes.cbegin()) {
                    json.append(",");
                }
                json.append("\"");
                json.append(string_utils::bin2hex(i->hash));
                json.append("\"");
            }
            json.append("]}}");

            get_dumps->set_host(settings::server::get_tor());
            get_dumps->set_body(json);
            get_dumps->reset_attempts();
            get_dumps->execute();
            response = get_dumps->get_response();
            if (!response) {
                LOGERR << "Cache. Could not get response from get-dumps-blocks-by-hash";
                continue;
            }
            if (response->get().body().empty()) {
                LOGERR << "Cache. get-dumps-blocks-by-hash empty response";
                continue;
            }

            if (reader.parse(response->get().body().c_str(), response->get().body().size())) {
                tmp = reader.get_error();
                if (tmp) {
                    LOGERR << "Cache. get-dumps-blocks-by-hash got error: " << reader.stringify(tmp);
                    continue;
                }
                tmp = reader.get_result();
                if (tmp) {
                    LOGERR << "Cache. get-dumps-blocks-by-hash got result " << reader.stringify(tmp);
                    continue;
                }
                LOGWARN << "Cache. get-dumps-blocks-by-hash body has parsed " << response->get().body().size() << " bytes";
            }

            decompressed = torrent_node_lib::decompress(response->get().body());
            resp_size = decompressed.size();

            LOGINFO << "Cache. Getting " << hashes.size() << " block(s). Size compressed " << response->get().body().size() << "/ decompressed " << resp_size;

            if (resp_size == 0) {
                continue;
            }

            buf = decompressed.c_str();
            p = buf;
            iter = hashes.begin();

            // reset previous block
            bi_prev.header.hash.clear();

            while (p < buf + resp_size && iter != hashes.end()) {
                memcpy(&blk_size, p, sizeof(blk_size));
                blk_size = __builtin_bswap64(blk_size);
                if (blk_size == 0) {
                    LOGERR << "Cache. Error while response parse";
                    break;
                }
                p += sizeof(blk_size);
                memset(ext_data, 0, sizeof(ext_data));
                tmp_blk_num = iter->number;
                if (settings::system::blocks_cache_block_verification && tmp_blk_num > 1/* && tmp_blk_num <= extra_blocks_epoch*/) {
                    // getting previous block
                    if (bi_prev.header.hash.empty()) {
                        // try get from cache
                        tmp_str.clear();
                        if (get_block_by_num(tmp_blk_num - 1, tmp_str)) {
                            bi_prev = std::get<torrent_node_lib::BlockInfo>(torrent_node_lib::Sync::parseBlockDump(tmp_str, false));
                            bi_prev.header.blockNumber = tmp_blk_num - 1;
                        } else {
                            // if block does not exist in cache then take it from torrent
                            json.clear();
                            string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-block-by-number\", \"params\":{\"number\":",
                                                         std::to_string(tmp_blk_num - 1), "}}");
                            get_block->set_host(settings::server::get_tor());
                            get_block->set_body(json);
                            get_block->reset_attempts();
                            get_block->execute();
                            response = get_block->get_response();
                            if (!response) {
                                LOGERR << "Cache. Could not get response from get-block-by-number";
                                break;
                            }
                            if (response->get().body().empty()) {
                                LOGERR << "Cache. Could not get get-block-by-number";
                                break;
                            }
                            if (!reader.parse(response->get().body().c_str(), response->get().body().size())) {
                                LOGERR << "Cache. Could not parse get-block-by-number (" << reader.get_parse_error() << "): " << reader.get_parse_error_str();
                                break;
                            }
                            tmp = reader.get_error();
                            if (tmp != nullptr) {
                                LOGERR << "Cache. Got error from get-block-by-number: " << reader.stringify(tmp);
                                break;
                            }
                            tmp = reader.get_result();
                            if (tmp == nullptr) {
                                LOGERR << "Cache. Did not find result in get-block-by-number";
                                break;
                            }
                            tmp_str.clear();
                            if (!reader.get_value(*tmp, "hash", tmp_str)) {
                                LOGERR << "Cache. Did not find hash in result get-block-by-number";
                                break;
                            }
                            bi_prev.header.hash = common::fromHex(tmp_str);
                            if (bi_prev.header.hash.empty()) {
                                LOGERR << "Cache. Hash must be not blank";
                                break;
                            }
                            bi_prev.header.blockNumber = tmp_blk_num - 1;
                        }
                    }

                    // getting current block
                    bi = std::get<torrent_node_lib::BlockInfo>(torrent_node_lib::Sync::parseBlockDump(std::string(p, blk_size), false));
                    bi.header.blockNumber = tmp_blk_num;

                    // checking hashes
                    if (bi.header.hash != iter->hash) {
                        LOGERR << "Cache. Block " << string_utils::bin2hex(bi.header.hash) << " is not equal " << string_utils::bin2hex(iter->hash);
                        break;
                    }

                    // checking current block core addresses
                    int verify = core_addr_verification(bi, string_utils::bin2hex(bi_prev.header.hash));
                    if (verify == -1) {
                        dump_bad_block(bi.header.blockNumber.value(), p, blk_size);
                        break;
                    }

                    // save block without sign
                    if (!save_block(static_cast<blk_number>(bi.header.blockNumber.value()),
                                    string_utils::bin2hex(bi.header.hash),
                                    std::string_view(p, blk_size),
                                    std::string_view(ext_data, sizeof(ext_data)))) {
                        break;
                    }

                    m_nextblock = tmp_blk_num + 1;
                    update_number(m_nextblock);

                    if (verify > 0) {
                        if (bi_prev.header.blockNumber.has_value() && bi_prev.header.blockNumber.value() > 0) {
                            ext_data[0] = blk_signed;
                            save_extra_data(std::to_string(bi_prev.header.blockNumber.value()), std::string_view(ext_data, sizeof(ext_data)));
                            m_last_signed_block = bi_prev.header.blockNumber.value();
                            update_last_signed(m_last_signed_block);
                        }
                    } else if (verify == 0) {
                        ext_data[0] = auto_signed_block(tmp_blk_num) ? blk_signed : blk_not_signed;
                        save_extra_data(std::to_string(tmp_blk_num), std::string_view(ext_data, sizeof(ext_data)));
                        if (ext_data[0] == blk_signed) {
                            m_last_signed_block = tmp_blk_num;
                            update_last_signed(m_last_signed_block);
                        }
                    } else {
                        LOGWARN << "Block #" << tmp_blk_num << " does not match any conditions";
                    }

                    std::swap(bi, bi_prev);
                } else {
                    ext_data[0] = tmp_blk_num == 1 ? blk_signed : blk_not_checked;
                    if (save_block(tmp_blk_num,
                                   string_utils::bin2hex(iter->hash),
                                   std::string_view(p, blk_size),
                                   std::string_view(ext_data, sizeof(ext_data)))) {
                        m_nextblock = tmp_blk_num + 1;
                        update_number(m_nextblock);
                    } else {
                        break;
                    }
                }
                p += blk_size;
                iter++;
            }

            hashes.erase(iter, hashes.end());
            load_extra_blocks();
        }
    }
    CACHE_END()
}

bool blocks_cache::save_block(blk_number number, const std::string& hash, const std::string_view& dump, const std::string_view& ext_data)
{
    CACHE_BGN
    {
        std::string num = std::to_string(number);
        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, num, leveldb::Slice(dump.data(), dump.size()));
        if (status.ok()) {
            if (!hash.empty()) {
                status = m_db->Put(opt, leveldb::Slice(hash.data(), hash.size()), num);
                if (status.ok() && save_extra_data(num, ext_data)) {
                    LOGINFO << "Cache. Block #" << number << " (" << hash << ") has been saved. " << dump.size() << " bytes";
                    return true;
                }
            }
        }
        LOGERR << "Cache. Could not save Block #" << number;
        return false;
    }
    CACHE_END(return false)
}

bool blocks_cache::update_number(blk_number number)
{
    CACHE_BGN
    {
        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, "next_block", std::to_string(number));
        if (!status.ok()) {
            LOGERR << "Cache. Could not update next block number: " << number;
            return false;
        }
        return true;
    }
    CACHE_END(return false)
}

bool blocks_cache::update_last_signed(blk_number number)
{
    CACHE_BGN
    {
        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, "last_signed_block", std::to_string(number));
        if (!status.ok()) {
            LOGERR << "Cache. Could not update last signed block number: " << number;
            return false;
        }
        return true;
    }
    CACHE_END(return false)
}

bool blocks_cache::get_block_by_num(blk_number number, std::string& result) const
{
    CACHE_BGN
    {
        return get_block_by_num(std::to_string(number), result);
    }
    CACHE_END(return false)
}

bool blocks_cache::get_block_by_num(const std::string& number, std::string& result) const
{
    CACHE_BGN
    {
        leveldb::ReadOptions read_opt;
        if (settings::system::blocks_cache_block_verification) {
            std::string ext_data;
            leveldb::Status status = m_db->Get(read_opt, string_utils::str_concat(number, "@data"), &ext_data);
            if (status.ok() && ext_data[0] != blk_signed) {
                return false;
            }
        }
        return m_db->Get(read_opt, number, &result).ok();
    }
    CACHE_END(return false)
}

bool blocks_cache::get_block_by_hash(const std::string& hash, std::string& num, std::string& result) const
{
    CACHE_BGN
    {
        if (get_block_num_by_hash(hash, num) && !num.empty()) {
            return get_block_by_num(num, result);
        }
        return false;
    }
    CACHE_END(return false)
}

bool blocks_cache::get_block_num_by_hash(const std::string& hash, std::string& result) const
{
    CACHE_BGN
    {
        return m_db->Get(leveldb::ReadOptions(), hash, &result).ok();
    }
    CACHE_END(return false)
}

bool blocks_cache::get_extra_block_for(blk_number number, std::string& result) const
{
    CACHE_BGN
    {
        std::string key = string_utils::str_concat(std::to_string(number), "@extra");
        return m_db->Get(leveldb::ReadOptions(), key, &result).ok();
    }
    CACHE_END(return false)
}

int blocks_cache::core_addr_verification(const torrent_node_lib::BlockInfo& bi, const std::string& prev_hash)
{
    CACHE_BGN
    {
        if (bi.header.isStateBlock() || bi.header.isForgingBlock()) {
            return 1;
        }
        bool succ = false;
        std::vector<std::string> cores = settings::system::cores;
        std::vector<std::string>::iterator it;

        std::string from;
        for (size_t i = 0; i < bi.txs.size(); ++i) {
            if (i > 6) {
                break;
            }
            if (!bi.txs[i].isSignBlockTx) {
                LOGWARN << "Cache. Block " << string_utils::bin2hex(bi.header.hash) << " tx[" << i << "] is not a SignBlockTx";
                succ = true;
                continue;
            }
            succ = false;
            if (bi.txs[i].toAddress != bi.txs[i].fromAddress) {
                LOGERR << "Cache. Block " << string_utils::bin2hex(bi.header.hash) << " tx[" << i << "] fields fromAddress (" << bi.txs[i].fromAddress.calcHexString()
                       << ") and toAddress (" << bi.txs[i].toAddress.calcHexString() << ") is not equal";
                break;
            }
            from = bi.txs[i].fromAddress.calcHexString();
            std::transform(from.begin(), from.end(), from.begin(), ::tolower);
            for (it = cores.begin(); it != cores.end(); it++) {
                if (it->compare(from) == 0){
                    cores.erase(it);
                    succ = true;
                    break;
                }
            }
            if (!succ) {
                LOGERR << "Cache. Block #" << bi.header.blockNumber.value() << " " << string_utils::bin2hex(bi.header.hash) << " tx[" << i << "] " << from << " is not core address";
                break;
            }
            std::string tmp = string_utils::bin2hex(bi.txs[i].data);
            if (tmp.compare(prev_hash) != 0) {
                succ = false;
                LOGERR << "Cache. Block #" << bi.header.blockNumber.value() << " " << string_utils::bin2hex(bi.header.hash) << " tx[" << i << "] data " << tmp << " is not equal previous hash " << prev_hash ;
                break;
            }
        }
        if (succ) {
            return static_cast<int>(settings::system::cores.size() - cores.size());
        }
        LOGERR << "Cache. Block #" << bi.header.blockNumber.value() << " " << string_utils::bin2hex(bi.header.hash)
               << " did not pass verification (" << settings::system::cores.size() - cores.size() << "/" << settings::system::cores.size() << ")";
        return -1;
    }
    CACHE_END(return -1)
}

int blocks_cache::core_addr_verification(const torrent_node_lib::SignBlockInfo& bi)
{
    CACHE_BGN
    {
        bool succ = false;
        std::vector<std::string> cores = settings::system::cores;
        std::vector<std::string>::iterator it;

        std::string from;
        for (size_t i = 0; i < bi.txs.size(); ++i) {
            if (i > 6) {
                break;
            }
            succ = false;
            from = bi.txs[i].address.calcHexString();
            std::transform(from.begin(), from.end(), from.begin(), ::tolower);
            for (it = cores.begin(); it != cores.end(); it++) {
                if (it->compare(from) == 0){
                    cores.erase(it);
                    succ = true;
                    break;
                }
            }
            if (!succ) {
                LOGERR << "Cache. Extra block hash " << string_utils::bin2hex(bi.header.hash) << " tx[" << i << "] " << from << " is not core address";
                break;
            }
        }
        if (succ) {
            return static_cast<int>(settings::system::cores.size() - cores.size());
        }
        LOGERR << "Cache. Extra block hash " << string_utils::bin2hex(bi.header.hash) << " did not pass the verification (" << settings::system::cores.size() - cores.size() << "/" << settings::system::cores.size() << ")";
        return -1;
    }
    CACHE_END(return -1)
}

void blocks_cache::dump_bad_block(size_t num, const char* buf, size_t size)
{
    CACHE_BGN
    {
        if (!fs_utils::dir::is_exists("./data/bad_dumps/")){
            if (!fs_utils::dir::create("./data/bad_dumps/")) {
                LOGERR << "Cache. Could not create folder ./data/bad_dumps/";
                return;
            };
        }
        std::chrono::time_point now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char c[32] = {0};
        strftime(c, sizeof(c), "_%d_%m_%Y_%H_%M_%S.", std::localtime(&t));
        std::string name;
        string_utils::str_append(name, "./data/bad_dumps/", std::to_string(num), c, std::to_string(now.time_since_epoch().count()));
        std::ofstream f{name};
        f.write(buf, static_cast<std::streamsize>(size));
        f.flush();
        f.close();
    }
    CACHE_END()
}

bool blocks_cache::save_extra_data(const std::string& number, const std::string_view& dump)
{
    CACHE_BGN
    {
        std::string key = string_utils::str_concat(number, "@data");
        if (!m_db->Put(leveldb::WriteOptions(), key, leveldb::Slice(dump.data(), dump.size())).ok()) {
            LOGERR << "Cache. Could not save extra data for block number #" << number;
            return false;
        }
        return true;
    }
    CACHE_END(return false)
}

bool blocks_cache::save_extra_block(const std::string& number, const std::string_view& dump)
{
    CACHE_BGN
    {
        std::string key = string_utils::str_concat(number, "@extra");
        if (!m_db->Put(leveldb::WriteOptions(), key, leveldb::Slice(dump.data(), dump.size())).ok()) {
            LOGERR << "Cache. Could not save extra block #" << number;
            return false;
        }
        return true;
    }
    CACHE_END(return false)
}

bool blocks_cache::auto_signed_block(blk_number number) const
{
    for (const auto& v : auto_signed_blocks) {
        if (v.first == number) {
            return true;
        }
        if (v.first < v.second && v.first < number && number <= v.second) {
            return true;
        }
    }
    return false;
}
