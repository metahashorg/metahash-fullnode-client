#include "get_dump_block_base.h"
#include "settings/settings.h"
#include "../SyncSingleton.h"
#include "../generate_json.h"
#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"
#include "check.h"
#include "cache/blocks_cache.h"
#include "string_utils.h"

get_dump_block_base::get_dump_block_base(session_context_ptr ctx)
    : base_network_handler(settings::server::get_tor(), ctx)
    , m_fromByte(0)
    , m_toByte(std::numeric_limits<size_t>::max())
    , m_isHex(true)
    , m_from_cache(false)
{
}

bool get_dump_block_base::prepare_params()
{
    BGN_TRY
    {
        CHK_REQ(m_id, "id field not found");

        auto params = m_reader.get_params();
        CHK_REQ(params, "params field not found")

        set_block_id(params);

        if (!settings::system::useLocalDatabase) {
            m_writer.add_param("isHex", m_isHex);
        }

        m_reader.get_value(*params, "fromByte", m_fromByte);
        m_reader.get_value(*params, "toByte", m_toByte);

        if (blocks_cache::get()->runing()) {
            std::string dump;
            std::string num;
            if ((std::holds_alternative<std::size_t>(m_block_id) &&
                blocks_cache::get()->get_block_by_num(std::get<std::size_t>(m_block_id), dump)) ||
                (std::holds_alternative<std::string>(m_block_id) &&
                 blocks_cache::get()->get_block_by_hash(std::get<std::string>(m_block_id), num, dump)))
            {
                 m_writer.reset();
                 std::string hexdump;
                 string_utils::bin2hex(dump, hexdump);
                 genBlockDumpJson(hexdump, false, m_writer.get_doc());
                 m_from_cache = true;
            } else {
                std::size_t number = static_cast<std::size_t>(std::atoi(get_block_id().c_str()));
                CHK_PRM(number <= blocks_cache::extra_blocks_epoch, "The block does not exists or have not signed yet");
            }
        }

        return true;
    }
    END_TRY(return false)
}

void get_dump_block_base::execute()
{
    BGN_TRY
    {
        if (m_from_cache) {
            LOGINFO << "Get block #" << get_block_id() << " from cache";
            return;
        }
        if (settings::system::useLocalDatabase) {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            try {

                const torrent_node_lib::BlockHeader bh = std::holds_alternative<std::size_t>(m_block_id) ?
                            sync.getBlockchain().getBlock(std::get<std::size_t>(m_block_id)) :
                            sync.getBlockchain().getBlock(std::get<std::string>(m_block_id));

                CHECK(bh.blockNumber.has_value(), "block " + get_block_id() + " has not found");
                const std::string res = sync.getBlockDump(torrent_node_lib::CommonMimimumBlockHeader(bh.hash, bh.filePos), m_fromByte, m_toByte, m_isHex, false);

                CHECK(!res.empty(), "block " + get_block_id() + " not found");
                if (m_isHex) {
                    genBlockDumpJson(res, false, m_writer.get_doc());
                } else {
                    //return res;
                }
            } catch (const common::exception &e) {
                genErrorResponse(-32603, e, m_writer.get_doc());
            } catch (const std::exception &e) {
                genErrorResponse(-32603, e.what(), m_writer.get_doc());
            } catch (...) {
                genErrorResponse(-32603, "Unknown error", m_writer.get_doc());
            }
        } else {
            base_network_handler::execute();
        }
    }
    END_TRY()
}

std::string get_dump_block_base::get_block_id() const
{
    if (std::holds_alternative<std::size_t>(m_block_id)) {
        return std::to_string(std::get<std::size_t>(m_block_id));
    } else if (std::holds_alternative<std::string>(m_block_id)) {
        return std::get<std::string>(m_block_id);
    }
    return std::string("UNKNOWN");
}
