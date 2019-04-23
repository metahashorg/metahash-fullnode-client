#include "auto_cache.h"
#include "common/stopProgram.h"
#include "log.h"
#include "settings/settings.h"
#include "common/filesystem_utils.h"
#include "common/string_utils.h"
#include "json_rpc.h"
#include <boost/exception/all.hpp>

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

auto_cache::auto_cache()
    : m_run(false)
    , m_nextblock(0)
{
}

auto_cache::~auto_cache()
{
}

bool auto_cache::running() const
{
    return m_run;
}

bool auto_cache::start()
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

        leveldb::Options options;
        options.create_if_missing = true;

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
            LOGINFO << "Cache. Last known block is " << m_nextblock;
        }

        LOGINFO << "Cache. Init successfully";

        m_worker = std::make_unique<std::thread>(worker_proc, this);
        return true;
    }
    CACHE_END(false)
}

void auto_cache::stop()
{
    CACHE_BGN
    {
        m_run = false;
        if (m_worker->joinable()) {
            m_worker->join();
        }
    }
    CACHE_END()
}

void auto_cache::worker_proc(auto_cache *param)
{
    param->routine();
}

void auto_cache::routine()
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
        while (m_run) {
            tp = std::chrono::high_resolution_clock::now() + std::chrono::seconds(30);

            res = perform<get_count_blocks_handler>(nullptr, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-count-blocks\"}");
            if (res.message.empty()) {
                LOGERR << "Cache. Could not get get-blocks-count";
                goto next;
            }
            if (!reader.parse(res.message)) {
                LOGERR << "Cache. Could not parse get-blocks-count: " << reader.get_parse_error().Code();
                goto next;
            }
            tmp = reader.get_result();
            if (tmp == nullptr) {
                LOGERR << "Cache. Did not find result in get-blocks-count";
                goto next;
            }
            if (!reader.get_value(*tmp, "count_blocks", count_blocks)) {
                LOGERR << "Cache. Did not find field 'count_blocks' in get-blocks-count";
                goto next;
            }
            if (m_nextblock == 0) {
                m_nextblock = count_blocks - 1000;
            }

            while (m_run && m_nextblock <= count_blocks) {
                json.clear();
                string_utils::str_append(json, "{\"id\":1, \"version\":\"2.0\", \"method\":\"get-dump-block-by-number\", \"params\":{\"number\":", std::to_string(m_nextblock), "}}");
                res = perform<get_dump_block_by_number>(nullptr, json);

                if (res.message.empty()) {
                    LOGERR << "Cache. Could not get get-dump-block-by-number";
                    goto next;
                }
                if (!reader.parse(res.message)) {
                    LOGERR << "Cache. Could not parse get-dump-block-by-number: " << reader.get_parse_error().Code();
                    goto next;
                }
                tmp = reader.get_result();
                if (tmp == nullptr) {
                    LOGERR << "Cache. Did not find result in get-dump-block-by-number";
                    goto next;
                }
                if (!reader.get_value(*tmp, "dump", dump)) {
                    LOGERR << "Cache. Did not find field 'dump' in get-dump-block-by-numbert";
                    goto next;
                }

                save_block(m_nextblock++, dump);
                common::checkStopSignal();
            }

next:
            common::checkStopSignal();
            std::this_thread::sleep_until(tp);
        }
    }
    CACHE_END()
    LOGINFO << "Cache. Stoped";
}

void auto_cache::save_block(unsigned int number, const std::string_view& dump)
{
    CACHE_BGN
    {
        leveldb::WriteOptions opt;
        leveldb::Status status = m_db->Put(opt, std::to_string(number), leveldb::Slice(dump.data(), dump.size()));
        if (status.ok()) {
            LOGINFO << "Cache. Saved block " << number;
        } else {
            LOGERR << "Cache. Could not save block " << number;
        }
    }
    CACHE_END()
}
