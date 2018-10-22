#include "task_handlers.h"

// get_tx_handler
bool get_tx_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        std::string hash;
        CHK_PRM(m_reader.get_value(*params, "hash", hash), "hash field not found")
        CHK_PRM(!hash.empty(), "hash is empty")
        CHK_PRM(hash.compare(0, 2, "0x") == 0, "hash field incorrect format")

        m_writer.add_param("hash", hash.c_str());

        return true;
    }
    END_TRY_RET(false)
}
