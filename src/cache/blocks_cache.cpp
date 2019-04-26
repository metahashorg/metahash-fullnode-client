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

#include "task_handlers/get_count_blocks_handler.h"
#include "task_handlers/get_dump_block_by_number_handler.h"

#define CACHE_BGN try

#define CACHE_END(ret) \
    catch (const common::StopException&) {\
        return ret;\
    } catch (boost::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);\
        return ret;\
    } catch (std::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();\
        return ret;\
    } catch (...) {\
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";\
        return ret;\
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
    CACHE_END(false)
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
    param->routine();
}

void blocks_cache::routine()
{
    CACHE_BGN
    {
        LOGINFO << "Cache. Started";
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
                string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dump-block-by-number\", \"params\":{\"number\":", std::to_string(m_nextblock), "}}");
                gdbn->set_body(json);
                gdbn->set_host(settings::server::get_tor());
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
                        LOGERR << "Cache. get-count-blocks error: json response occured";
                    }
                    goto next;
                }
                if (save_block(m_nextblock, response->get().body())) {
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

bool blocks_cache::save_block(unsigned int number, const std::string& dump)
{
    CACHE_BGN
    {
        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, std::to_string(number), leveldb::Slice(dump.c_str(), dump.size()));
        if (status.ok()) {
            LOGINFO << "Cache. Block " << number << " has been saved. " << dump.size() << " bytes";
        } else {
            LOGERR << "Cache. Could not save block " << number;
            return false;
        }
        return true;
    }
    CACHE_END(false)
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
    CACHE_END(false)
}

bool blocks_cache::get_block(unsigned int number, std::string& result)
{
    CACHE_BGN
    {
        leveldb::ReadOptions opt;
        leveldb::Status status = m_db->Get(opt, std::to_string(number), &result);
        return status.ok();
    }
    CACHE_END(false)
}
