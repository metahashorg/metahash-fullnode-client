#include "get_block_by_number_handler.h"

bool get_block_by_number_handler::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")

        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")

        auto &jsonParams = *params;
        CHK_PRM(jsonParams.HasMember("number") && jsonParams["number"].IsInt64(), "number field not found")
        size_t number = jsonParams["number"].GetInt64();
        m_writer.add_param("number", number);
        
        uint32_t type(0);
        if (jsonParams.HasMember("type") && jsonParams["type"].IsInt64()) {
            type = jsonParams["type"].GetInt64();
            m_writer.add_param("type", type);
        }

        mh_count_t countTxs(0);
        if (jsonParams.HasMember("countTxs") && jsonParams["countTxs"].IsInt64()) {
            countTxs = jsonParams["countTxs"].GetInt64();
            m_writer.add_param("countTxs", countTxs);
        }
        
        mh_count_t beginTx(0);
        if (jsonParams.HasMember("beginTx") && jsonParams["beginTx"].IsInt64()) {
            beginTx = jsonParams["beginTx"].GetInt64();
            m_writer.add_param("beginTx", beginTx);
        }
        
        return true;
    }
    END_TRY_RET(false)
}
