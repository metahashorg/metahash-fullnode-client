#include <memory>
#include "task_handlers.h"
#include "../http_json_rpc_request.h"
#include "../wallet_storage/wallet_storage.h"
#include "../log/log.h"

#include "generate_handler.h"
#include "fetch_balance_handler.h"
#include "fetch_history_handler.h"
#include "get_block_by_hash_handler.h"
#include "get_block_by_number_handler.h"
#include "get_blocks_handler.h"
#include "get_count_blocks_handler.h"
#include "get_dump_block_by_hash.h"
#include "get_dump_block_by_number.h"
#include "get_tx_handler.h"
#include "get_last_txs_handler.h"
#include "send_tx_handler_sync.h"

template <class T>
void base_network_handler<T>::execute()
{
    BGN_TRY
    {
        this->m_request->set_path(this->m_reader.get_method());
        this->m_request->set_body(this->m_writer.stringify());

        this->m_result.pending = this->m_async_execute;
        if (!this->m_async_execute)
        {
            this->m_request->execute();
            this->m_writer.reset();
            this->m_writer.parse(this->m_request->get_result());
        }
        else
        {
            this->m_request->execute_async(boost::bind(&base_network_handler<T>::on_complete, this->shared_from_this(), this->m_id, this->m_request, this->m_session));
        }
    }
    END_TRY
}

template <class T>
void base_network_handler<T>::processResponse(json_rpc_id id, http_json_rpc_request_ptr req) {
    json_rpc_reader reader;
   
    CHK_PRM(reader.parse(req->get_result()), "Invalid response json")
    
    json_rpc_id _id = reader.get_id();
    
    //CHK_PRM(_id != 0 && _id == id, "Returned id doesn't match")
    
    auto err = reader.get_error();
    auto res = reader.get_result();
    
    CHK_PRM(err || res, "No occur result or error")
    
    if (err) {
        this->m_writer.set_error(*err);
    } else if (res) {
        this->m_writer.set_result(*res);
    }
    
    rapidjson::Document& doc = reader.get_doc();
    for (auto& m : doc.GetObject()) {
        std::string name = m.name.GetString();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (std::find(json_rpc_service.begin(), json_rpc_service.end(), name) != json_rpc_service.end()) {
            continue;
        }
        this->m_writer.add_value(m.name.GetString(), m.value);
    }
}

template <class T>
void base_network_handler<T>::on_complete(json_rpc_id id, http_json_rpc_request_ptr req, http_session_ptr session)
{
    BGN_TRY
    {
        this->m_writer.reset();
        
        processResponse(id, req);

        boost::asio::post(boost::bind(&http_session::send_json, session, this->m_writer.stringify()));
    }
    END_TRY_PARAM(boost::asio::post(boost::bind(&http_session::send_json, session, this->m_writer.stringify())))
}

template class base_network_handler<create_tx_handler>;
template class base_network_handler<send_tx_handler>;
template class base_network_handler<send_tx_handler_sync>;
template class base_network_handler<fetch_balance_handler>;
template class base_network_handler<fetch_history_handler>;
template class base_network_handler<get_dump_block_by_hash>;
template class base_network_handler<get_dump_block_by_number>;
template class base_network_handler<get_block_by_hash_handler>;
template class base_network_handler<get_block_by_number_handler>;
template class base_network_handler<get_tx_handler>;
template class base_network_handler<get_blocks_handler>;
template class base_network_handler<get_count_blocks_handler>;
template class base_network_handler<get_last_txs_handler>;
