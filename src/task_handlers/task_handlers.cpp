#include "task_handlers.h"

#include "create_tx_handler.h"
#include "send_tx_handler.h"
#include "generate_handler.h"
#include "fetch_balance_handler.h"
#include "fetch_history_handler.h"
#include "get_block_by_hash_handler.h"
#include "get_block_by_number_handler.h"
#include "get_blocks_handler.h"
#include "get_count_blocks_handler.h"
#include "get_dump_block_by_hash_handler.h"
#include "get_dump_block_by_number_handler.h"
#include "get_tx_handler.h"
#include "get_last_txs_handler.h"
#include "status_handler.h"
#include "fetch_transaction_handler.h"
#include "addr_validate_handler.h"

#include "fetch_balance_handler_sync.h"
#include "fetch_history_handler_sync.h"
#include "send_tx_handler_sync.h"
#include "get_count_blocks_handler_sync.h"
#include "get_block_by_hash_handler_sync.h"
#include "get_block_by_number_handler_sync.h"
#include "get_tx_handler_sync.h"
#include "get_dump_block_by_hash_handler_sync.h"
#include "get_dump_block_by_number_handler_sync.h"

const std::map<std::pair<std::string, UseLocalDatabase>, handler_func> post_handlers = {
    { std::pair<std::string, UseLocalDatabase>("generate", false),                  perform<generate_handler> },
    { std::pair<std::string, UseLocalDatabase>("create-tx", false),                 perform<create_tx_handler> },
    { std::pair<std::string, UseLocalDatabase>("create-tx2", false),                perform<create_tx_handler_v2> },
    { std::pair<std::string, UseLocalDatabase>("send-tx", false),                   perform<send_tx_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-count-blocks", false),          perform<get_count_blocks_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-last-txs", false),              perform<get_last_txs_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-block-by-hash", false),         perform<get_block_by_hash_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-block-by-number", false),       perform<get_block_by_number_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-blocks", false),                perform<get_blocks_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-dump-block-by-number", false),  perform<get_dump_block_by_number> },
    { std::pair<std::string, UseLocalDatabase>("get-dump-block-by-hash", false),    perform<get_dump_block_by_hash> },
    { std::pair<std::string, UseLocalDatabase>("get-tx", false),                    perform<get_tx_handler> },
    { std::pair<std::string, UseLocalDatabase>("fetch-balance", false),             perform<fetch_balance_handler> },
    { std::pair<std::string, UseLocalDatabase>("fetch-history", false),             perform<fetch_history_handler> },
    { std::pair<std::string, UseLocalDatabase>("status", false),                    perform<status_handler> },
    { std::pair<std::string, UseLocalDatabase>("fetch-transaction", false),         perform<fetch_transaction_handler> },
    { std::pair<std::string, UseLocalDatabase>("validate", false),                  perform<addr_validate_handler> },

    { std::pair<std::string, UseLocalDatabase>("generate", true),                   perform<generate_handler> },
    { std::pair<std::string, UseLocalDatabase>("create-tx", true),                  perform<create_tx_handler> },
    { std::pair<std::string, UseLocalDatabase>("create-tx2", true),                 perform<create_tx_handler_v2> },
    { std::pair<std::string, UseLocalDatabase>("fetch-balance", true),              perform<fetch_balance_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("fetch-history", true),              perform<fetch_history_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("send-tx", true),                    perform<send_tx_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("get-count-blocks", true),           perform<get_count_blocks_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("get-last-txs", true),               perform<get_last_txs_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-block-by-hash", true),          perform<get_block_by_hash_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("get-block-by-number", true),        perform<get_block_by_number_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("get-blocks", true),                 perform<get_blocks_handler> },
    { std::pair<std::string, UseLocalDatabase>("get-tx", true),                     perform<get_tx_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("get-dump-block-by-hash", true),     perform<get_dump_block_by_hash_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("get-dump-block-by-number", true),   perform<get_dump_block_by_number_handler_sync> }, // +
    { std::pair<std::string, UseLocalDatabase>("status", true),                     perform<status_handler> },
    { std::pair<std::string, UseLocalDatabase>("fetch-transaction", true),          perform<fetch_transaction_handler> },
    { std::pair<std::string, UseLocalDatabase>("validate", true),                   perform<addr_validate_handler> }
};

const std::map<std::string_view, handler_func> get_handlers = {
    { "status",     perform<status_handler> }
};
