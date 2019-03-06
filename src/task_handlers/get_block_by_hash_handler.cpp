#include "get_block_by_hash_handler.h"

bool get_block_by_hash_handler::prepare_params()
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

        auto &jsonParams = *params;
        
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

void get_block_by_hash_handler::processResponse(json_rpc_id id, json_rpc_reader &reader)
{
    base_network_handler::processResponse(id, reader);
    if (!settings::system::allowStateBlocks) {
        auto res = reader.get_result();
        if (res) {
            std::string type;
            CHK_PRM(reader.get_value(*res, "type", type), "'type' field not found");
            CHK_PRM(type.compare("state") != 0, "block is state-block and has been ignored");
        }
    }
}
