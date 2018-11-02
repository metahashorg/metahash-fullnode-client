#include "send_tx_handler_sync.h"

#include "../SyncSingleton.h"
#include "../sync/BlockInfo.h"

bool send_tx_handler_sync::prepare_params()
{
    BGN_TRY
    {
        if (!this->check_params())
            return false;

        auto params = this->m_reader.get_params();
        auto jValue = this->m_reader.get("nonce", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "nonce field incorrect format")
            this->m_nonce = std::stoull(tmp);
        } else {
            CHK_PRM(syncSingleton() != nullptr, "Sync not set");
            const Sync &sync = *syncSingleton();
            
            const BalanceInfo balance = sync.getBalance(Address(this->m_address));
            this->m_nonce = balance.countSpent;
        }

        if (!this->build_request())
            return false;

        return true;
    }
    END_TRY_RET(false)
}

void send_tx_handler_sync::processResponse(json_rpc_id id, http_json_rpc_request_ptr req) {
    json_rpc_reader reader;
    
    CHK_PRM(reader.parse(req->get_result()), "Invalid response json")
    
    json_rpc_id _id = reader.get_id();
    
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
