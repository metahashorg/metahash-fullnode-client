#include "task_handlers.h"

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
#include "fetch_balance_handler_sync.h"
#include "fetch_history_handler_sync.h"
#include "send_tx_handler_sync.h"

template <class T>
bool base_handler<T>::prepare(const std::string& params)
{
    BGN_TRY
    {
        m_duration.start();

        CHK_PRM(this->m_reader.parse(params), "Parse error");

        this->m_id = this->m_reader.get_id();
        this->m_writer.set_id(this->m_id);

        const bool complete = this->prepare_params();
        const bool pending = this->m_result.pending;
        if (!complete && !pending)
        {
            // prepare_params must set an error

            if (!this->m_writer.is_error())
            {
                this->m_writer.reset();
                this->m_writer.set_error(-32602, "Invalid params");
            }
        }

        STREAM_LOG_DBG("Prepared json (" << complete << pending << "):" << std::endl << this->m_writer.stringify())

        return complete;
    }
    END_TRY_RET(false)
}

template class base_handler<create_tx_handler>;
template class base_handler<send_tx_handler>;
template class base_handler<fetch_balance_handler>;
template class base_handler<fetch_history_handler>;
template class base_handler<get_dump_block_by_hash>;
template class base_handler<get_dump_block_by_number>;
template class base_handler<get_block_by_hash_handler>;
template class base_handler<get_block_by_number_handler>;
template class base_handler<get_tx_handler>;
template class base_handler<get_blocks_handler>;
template class base_handler<get_count_blocks_handler>;
template class base_handler<get_last_txs_handler>;
template class base_handler<generate_handler>;
template class base_handler<fetch_balance_handler_sync>;
template class base_handler<fetch_history_handler_sync>;
template class base_handler<send_tx_handler_sync>;
