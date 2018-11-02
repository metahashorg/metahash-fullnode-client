#include "sync_handler.h"
#include <memory>
#include "task_handlers.h"
#include "../http_json_rpc_request.h"
#include "../wallet_storage/wallet_storage.h"
#include "../log/log.h"

#include "fetch_balance_handler_sync.h"
#include "fetch_history_handler_sync.h"
#include "send_tx_handler_sync.h"

template <class T>
void base_sync_handler<T>::execute()
{
    BGN_TRY
    {
        executeImpl();
    }
    END_TRY
}

template class base_handler<fetch_balance_handler_sync>;
template class base_handler<fetch_history_handler_sync>;
template class base_handler<send_tx_handler_sync>;
