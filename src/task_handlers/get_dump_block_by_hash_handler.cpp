#include "get_dump_block_by_hash_handler.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "check.h"
#include "cache/blocks_cache.h"
#include "string_utils.h"

extern std::unique_ptr<blocks_cache> g_cache;

get_dump_block_by_hash::get_dump_block_by_hash(http_session_ptr session)
    : base_network_handler(settings::server::get_tor(), session)
    , m_fromByte(0)
    , m_toByte(std::numeric_limits<size_t>::max())
    , m_isHex(true)
    , m_from_cache(false)
{
    m_duration.set_message(__func__);
}

bool get_dump_block_by_hash::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        CHK_PRM(m_reader.get_value(*params, "hash", m_hash), "hash field not found")
        CHK_PRM(!m_hash.empty(), "hash is empty")

        m_reader.get_value(*params, "fromByte", m_fromByte);
        m_reader.get_value(*params, "toByte", m_toByte);

        if (g_cache && g_cache->runing()) {
            std::string dump;
            std::string num;
            if (g_cache->get_block_by_hash(m_hash, num, dump)) {
                 std::string hexdump;
                 string_utils::bin2hex(dump, hexdump);
                 genBlockDumpJson(hexdump, false, m_writer.getDoc());
                 m_from_cache = true;
                 return true;
            }
        }

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("hash", m_hash);
            m_writer.add_param("isHex", m_isHex);
        }

        return true;
    }
    END_TRY_RET(false)
}

void get_dump_block_by_hash::execute()
{
    BGN_TRY
    {
        if (m_from_cache) {
            LOGDEBUG << "Get block " << m_hash << " from cache";
            return;
        }
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            try {
                const torrent_node_lib::BlockHeader bh = sync.getBlockchain().getBlock(m_hash);
                CHECK(bh.blockNumber.has_value(), "block " + m_hash + " has not found");
                const std::string res = sync.getBlockDump(bh, m_fromByte, m_toByte, m_isHex);

                CHECK(!res.empty(), "block " + m_hash + " not found");
                if (m_isHex) {
                    genBlockDumpJson(res, false, m_writer.getDoc());
                } else {
                    //return res;
                }
            } catch (const common::exception &e) {
                genErrorResponse(-32603, e, m_writer.getDoc());
            } catch (const std::exception &e) {
                genErrorResponse(-32603, e.what(), m_writer.getDoc());
            } catch (...) {
                genErrorResponse(-32603, "Unknown error", m_writer.getDoc());
            }
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY_RET()
}
