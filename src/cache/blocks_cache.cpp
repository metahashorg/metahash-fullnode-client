#include "blocks_cache.h"
#include "common/stopProgram.h"
#include "log.h"
#include "settings/settings.h"
#include "common/filesystem_utils.h"
#include "common/string_utils.h"
#include "json_rpc.h"
#include "http_json_rpc_request.h"
#include <boost/exception/all.hpp>
#include <boost/asio/io_context.hpp>
#include <byteswap.h>
#include "../sync/BlockInfo.h"
#include "../sync/synchronize_blockchain.h"
#include "compress.h"

#include "task_handlers/get_count_blocks_handler.h"
#include "task_handlers/get_dump_block_by_number_handler.h"

#define CACHE_BGN try

#define CACHE_END(ret) \
    catch (const common::StopException&) {\
        ret;\
    } catch (boost::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);\
        ret;\
    } catch (std::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();\
        ret;\
    } catch (...) {\
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";\
        ret;\
    }

blocks_cache::blocks_cache()
    : m_run(false)
    , m_nextblock(0)
{
}

blocks_cache::~blocks_cache()
{
}

bool blocks_cache::start()
{
    CACHE_BGN
    {
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
            m_nextblock = std::atoi(result.c_str());
        }
        LOGINFO << "Cache. Next block is " << m_nextblock;
        LOGINFO << "Cache. Init successfully";

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

unsigned int blocks_cache::next_block() const
{
    return m_nextblock;
}

void blocks_cache::worker_proc(blocks_cache *param)
{
    switch (settings::system::blocks_cache_ver) {
    case 2:
        param->routine_2();
        break;
    default:
        param->routine();
        break;
    }
}

void blocks_cache::routine()
{
    CACHE_BGN
    {
        LOGINFO << "Cache. Version 1 Started";
        m_run = true;
        unsigned int count_blocks = 0;
        handler_result res;
        std::string json;
        std::string_view dump;
        json_rpc_reader reader;
        rapidjson::Value* tmp = nullptr;
        std::chrono::system_clock::time_point tp;

        json_response_type* response = nullptr;
        boost::asio::io_context ctx;

        http_json_rpc_request_ptr gcb = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), ctx);
        gcb->set_path("get-count-blocks");
        gcb->set_body("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-count-blocks\"}");

        http_json_rpc_request_ptr gdbn = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), ctx);
        gdbn->set_path("get-dump-block-by-number");

        while (m_run) {
            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(30);

            ctx.restart();
            gcb->set_host(settings::server::get_tor());
            gcb->reset_attempts();
            gcb->execute();
            response = gcb->get_response();
            if (!response) {
                LOGERR << "Cache. Could not get response from get-count-blocks";
                goto next;
            }
            if (response->get().body().empty()) {
                LOGERR << "Cache. Could not get get-count-blocks";
                goto next;
            }
            if (!reader.parse(response->get().body())) {
                LOGERR << "Cache. Could not parse get-count-blocks: " << reader.get_parse_error().Code();
                goto next;
            }
            tmp = reader.get_result();
            if (tmp == nullptr) {
                LOGERR << "Cache. Did not find result in get-count-blocks";
                goto next;
            }
            if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                LOGERR << "Cache. Did not find field 'count_blocks' in get-count-blocks";
                goto next;
            }
            if (m_nextblock == 0) {
                m_nextblock = count_blocks - 50000;
            }

            while (m_run && m_nextblock <= count_blocks) {
                ctx.restart();
                json.clear();
                string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dump-block-by-number\", \"params\":{\"number\":", std::to_string(m_nextblock), ", \"compress\":true}}");
                gdbn->set_body(json);
                gdbn->set_host(settings::server::get_tor());
                gdbn->reset_attempts();
                gdbn->execute();
                response = gdbn->get_response();
                if (response->get().body().empty()) {
                    LOGERR << "Cache. Could not get get-dump-block-by-number";
                    goto next;
                }
                if (reader.parse(response->get().body())) {
                    tmp = reader.get_error();
                    if (tmp) {
                        LOGERR << "Cache. get-count-blocks error: " << reader.stringify(tmp);
                    } else {
                        LOGERR << "Cache. get-count-blocks error: json response occured " << reader.stringify();
                    }
                    goto next;
                }

                std::string buf = torrent_node_lib::decompress(response->get().body());
                torrent_node_lib::BlockInfo bi = torrent_node_lib::Sync::parseBlockDump(buf, false);

                if (save_block(m_nextblock, bi.header.hash, response->get().body())) {
                    update_number(++m_nextblock);
                }
                common::checkStopSignal();
            }

next:
            common::checkStopSignal();
            std::this_thread::sleep_until(tp);
        }
    }
    CACHE_END()
    m_run = false;
    LOGINFO << "Cache. Stoped";
}

void blocks_cache::routine_2()
{
    CACHE_BGN
    {
        LOGINFO << "Cache. Version 2 Started";
        m_run = true;
        handler_result res;
        std::string json;
        std::string decompressed;
        std::string_view dump;
        json_rpc_reader reader;
        rapidjson::Value* tmp = nullptr;
        rapidjson::Value::MemberIterator it;
        rapidjson::Value::Array::ValueIterator arr_it;
        std::chrono::system_clock::time_point tp;

        struct block_info {
            block_info(unsigned int number_, const char* data, size_t size)
                : number(number_)
                , hash(data, size) {}
            unsigned int number;
            std::string_view hash;
        };

        std::vector<block_info> hashes;
        std::vector<block_info>::const_iterator iter;
        const char
                *buf = nullptr,
                *p = nullptr;
        uint64_t blk_size = 0;
        size_t resp_size = 0;

        json_response_type* response = nullptr;
        boost::asio::io_context ctx;

        unsigned int tmp_blk_num = 0;
        if (m_nextblock == 0) {
            m_nextblock = 1;
            http_json_rpc_request_ptr get_count = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), ctx);
            get_count->set_path("get-count-blocks");
            get_count->set_body("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-count-blocks\"}");
            get_count->execute();
            response = get_count->get_response();
            for (;;) {
                if (!response) {
                    LOGERR << "Cache. Could not get response from get-count-blocks";
                    break;
                }
                if (response->get().body().empty()) {
                    LOGERR << "Cache. Could not get get-count-blocks";
                    break;
                }
                if (!reader.parse(response->get().body())) {
                    LOGERR << "Cache. Could not parse get-count-blocks: " << reader.get_parse_error().Code();
                    break;
                }
                tmp = reader.get_result();
                if (tmp == nullptr) {
                    LOGERR << "Cache. Did not find result in et-count-blocks";
                    break;
                }
                unsigned int count_blocks = 0;
                if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                    LOGERR << "Cache. Did not find field 'count_blocks' in get-count-blocks";
                    break;
                }
                if (count_blocks > settings::system::blocks_cache_init_count) {
                    m_nextblock = count_blocks - settings::system::blocks_cache_init_count;
                }
                break;
            }
            LOGINFO << "Cache. Next block set to " << m_nextblock;
        }

        // max summary size of blocks per request
        const unsigned int blocks_max_size = settings::system::blocks_cache_recv_data_size * 1024 * 1024 * 2; // because of compress we can increase approximately up to 2 times
        unsigned int blocks_size = 0;

        http_json_rpc_request_ptr get_blocks = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), ctx);
        get_blocks->set_path("get-blocks");

        http_json_rpc_request_ptr get_dumps = std::make_shared<http_json_rpc_request>(settings::server::get_tor(), ctx);
        get_dumps->set_path("get-dumps-blocks-by-hash");

        while (m_run) {
            common::checkStopSignal();

            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(30);

            json.clear();
            string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-blocks\", \"params\":{\"beginBlock\":",
                std::to_string(m_nextblock), ", \"countBlocks\": ", std::to_string(settings::system::blocks_cache_recv_count),
                ", \"type\": \"forP2P\", \"direction\": \"forward\"}}");

            ctx.restart();
            get_blocks->set_host(settings::server::get_tor());
            get_blocks->set_body(json);
            get_blocks->reset_attempts();
            get_blocks->execute();
            response = get_blocks->get_response();
            if (!response) {
                LOGERR << "Cache. Could not get response from get-blocks";
                goto wait;
            }
            if (response->get().body().empty()) {
                LOGERR << "Cache. Could not get get-blocks";
                goto wait;
            }
            if (!reader.parse(response->get().body())) {
                LOGERR << "Cache. Could not parse get-blocks: " << reader.get_parse_error().Code();
                goto wait;
            }
            tmp = reader.get_result();
            if (tmp == nullptr) {
                LOGERR << "Cache. Did not find result in get-blocks";
                goto wait;
            }
            if (!tmp->IsArray()) {
                LOGERR << "Cache. Expect array from get-blocks";
                goto wait;
            }
            if (tmp->Size() == 0) {
                goto wait;
            }
            hashes.clear();
            blocks_size = 0;
            for (arr_it = tmp->GetArray().begin(); arr_it != tmp->GetArray().end(); arr_it++) {
                if (!arr_it->IsObject()) {
                    continue;
                }
                it = arr_it->FindMember("size");
                if (it != arr_it->MemberEnd()) {
                    blocks_size += it->value.GetUint();
                    if (arr_it != tmp->GetArray().begin()) {
                        if (blocks_size > blocks_max_size) {
                            LOGWARN << "Cache. Reached maximum blocks data size per request: " << blocks_max_size << " bytes";
                            break;
                        }
                    }
                }
                it = arr_it->FindMember("number");
                if (it == arr_it->MemberEnd()) {
                    LOGERR << "Cache. Could not get block number from get-blocks";
                    goto wait;
                }
                tmp_blk_num = it->value.GetUint();
                it = arr_it->FindMember("hash");
                if (it == arr_it->MemberEnd()) {
                    LOGERR << "Cache. Could not get block hash from get-blocks";
                    goto wait;
                }
                hashes.emplace_back(tmp_blk_num, it->value.GetString(), it->value.GetStringLength());
            }

            if (hashes.empty()) {
                goto wait;
            }

            json.clear();
            json.append("{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dumps-blocks-by-hash\", \"params\":{\"compress\":true, \"hashes\":[");
            for (auto i = hashes.cbegin(); i != hashes.cend(); i++) {
                if (i != hashes.cbegin()) {
                    json.append(",");
                }
                json.append("\"");
                json.append(i->hash.data(), i->hash.size());
                json.append("\"");
            }
            json.append("]}}");

            ctx.restart();
            get_dumps->set_host(settings::server::get_tor());
            get_dumps->set_body(json);
            get_dumps->reset_attempts();
            get_dumps->execute();
            response = get_dumps->get_response();
            if (!response) {
                LOGERR << "Cache. Could not get response from get-dumps-blocks-by-hash";
                goto wait;
            }
            if (response->get().body().empty()) {
                LOGERR << "Cache. get-dumps-blocks-by-hash empty response";
                goto wait;
            }

            if (reader.parse(response->get().body())) {
                tmp = reader.get_error();
                if (tmp) {
                    LOGERR << "Cache. get-dumps-blocks-by-hash error: " << reader.stringify(tmp);
                } else {
                    LOGERR << "Cache. get-dumps-blocks-by-hash: json response occured " << reader.stringify();
                }
                goto wait;
            }

            decompressed = torrent_node_lib::decompress(response->get().body());
            resp_size = decompressed.size();

            LOGINFO << "Cache. Getting " << hashes.size() << " block(s). Size " << response->get().body().size() << "/" << resp_size;

            buf = decompressed.c_str();
            p = buf;
            iter = hashes.cbegin();
            while (p < buf + resp_size && iter != hashes.cend()) {
                memcpy(&blk_size, p, sizeof(blk_size));
                blk_size = __builtin_bswap64(blk_size);
                if (blk_size == 0) {
                    LOGERR << "Cache. Error while response parse";
                    break;
                }
                p += sizeof(blk_size);
                if (save_block(iter->number, iter->hash, std::string_view(p, blk_size))) {
                    m_nextblock = iter->number+1;
                    update_number(m_nextblock);
                }
                p += blk_size;
                iter++;
            }
            if (settings::system::blocks_cache_force) {
                continue;
            }

wait:
            common::checkStopSignal();
            std::this_thread::sleep_until(tp);
        }
    }
    CACHE_END()
    m_run = false;
    LOGINFO << "Cache. Stoped";
}

bool blocks_cache::save_block(unsigned int number, const std::string& dump)
{
    return save_block(number, "", std::string_view(dump.c_str(), dump.size()));
}

bool blocks_cache::save_block(unsigned int number, const std::string_view& hash, const std::string_view& dump)
{
    CACHE_BGN
    {
        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, std::to_string(number), leveldb::Slice(dump.data(), dump.size()));
        if (status.ok()) {
            LOGINFO << "Cache. Block #" << number << " (" << hash << ") has been saved. " << dump.size() << " bytes";
            if (!hash.empty()) {
                status = m_db->Put(opt, leveldb::Slice(hash.data(), hash.size()), std::to_string(number));
                if (!status.ok()) {
                    LOGERR << "Cache. Could not save hash for block " << number;
                }
            }
        } else {
            LOGERR << "Cache. Could not save block " << number;
            return false;
        }
        return true;
    }
    CACHE_END(return false)
}

bool blocks_cache::update_number(unsigned int number)
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

bool blocks_cache::get_block_by_num(unsigned int number, std::string& result)
{
    CACHE_BGN
    {
        leveldb::ReadOptions opt;
        leveldb::Status status = m_db->Get(opt, std::to_string(number), &result);
        return status.ok();
    }
    CACHE_END(return false)
}

bool blocks_cache::get_block_by_num(std::string& number, std::string& result)
{
    CACHE_BGN
    {
        leveldb::ReadOptions opt;
        leveldb::Status status = m_db->Get(opt, number, &result);
        return status.ok();
    }
    CACHE_END(return false)
}

bool blocks_cache::get_block_by_hash(std::string& hash, std::string& num, std::string& result)
{
    CACHE_BGN
    {
        leveldb::ReadOptions opt;
        leveldb::Status status = m_db->Get(opt, hash, &num);
        if (status.ok() && !num.empty()) {
            return get_block_by_num(num, result);
        }
        return false;
    }
    CACHE_END(return false)
}
