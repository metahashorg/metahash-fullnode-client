#include "get_dump_block_by_hash.h"

bool get_dump_block_by_hash::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        std::string hash;
        CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
        CHK_PRM(!hash.empty(), "hash is empty")

        m_writer.add_param("hash", hash);
        m_writer.add_param("isHex", true);

        return true;
    }
    END_TRY_RET(false)
}
