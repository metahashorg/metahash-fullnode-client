#include "send_tx_handler_sync.h"

#include "../SyncSingleton.h"
#include "../sync/BlockInfo.h"

bool send_tx_handler_sync::prepare_params()
{
    BGN_TRY
    {
        if (!check_params())
            return false;

        auto params = m_reader.get_params();
        auto jValue = m_reader.get("nonce", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "nonce field incorrect format")
            m_nonce = std::stoull(tmp);
        } else {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const torrent_node_lib::Sync &sync = *syncSingleton();

            const torrent_node_lib::BalanceInfo balance = sync.getBalance(torrent_node_lib::Address(m_address));
            m_nonce = balance.countSpent + 1;
        }

        if (!build_request())
            return false;

        return true;
    }
    END_TRY_RET(false)
}

void send_tx_handler_sync::process_response(json_rpc_reader &reader)
{
    BGN_TRY
    {
        //json_rpc_id _id = reader.get_id();
        //CHK_PRM(_id != 0 && _id == id, "Returned id doesn't match")

        auto err = reader.get_error();
        auto res = reader.get_result();
        auto params = reader.get_params();

        CHK_PRM(err || res, "No occur result or error")

        if (err) {
            this->m_writer.set_error(*err);
        } else {
            CHK_PRM(params->IsString(), "params field not found");
            this->m_writer.set_result(*params);
        }
    }
    END_TRY_RET();
}
