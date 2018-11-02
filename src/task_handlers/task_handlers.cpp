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
#include "get_dump_block_by_hash.h"
#include "get_dump_block_by_number.h"
#include "get_tx_handler.h"
#include "get_last_txs_handler.h"

#include "fetch_balance_handler_sync.h"
#include "fetch_history_handler_sync.h"
#include "send_tx_handler_sync.h"

const std::map<std::pair<std::string, UseLocalDatabase>, handler_func> map_handlers = {
    { std::pair<std::string, UseLocalDatabase>("generate", false),					generate_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("create-tx", false),					create_tx_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("send-tx", false),					send_tx_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("get-count-blocks", false),			get_count_blocks_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("get-last-txs", false),				get_last_txs_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("get-block-by-hash", false),			get_block_by_hash_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("get-block-by-number", false),		get_block_by_number_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("get-blocks", false),					get_blocks_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("get-dump-block-by-number", false),	get_dump_block_by_number::perform },
    { std::pair<std::string, UseLocalDatabase>("get-dump-block-by-hash", false),		get_dump_block_by_hash::perform },
    { std::pair<std::string, UseLocalDatabase>("get-tx", false),						get_tx_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("fetch-balance", false),				fetch_balance_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("fetch-history", false),				fetch_history_handler::perform },
    
    { std::pair<std::string, UseLocalDatabase>("generate", true),					generate_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("create-tx", true),					create_tx_handler::perform },
    { std::pair<std::string, UseLocalDatabase>("fetch-balance", true),				fetch_balance_handler_sync::perform },
    { std::pair<std::string, UseLocalDatabase>("fetch-history", true),				fetch_history_handler_sync::perform },
    { std::pair<std::string, UseLocalDatabase>("send-tx", true),					send_tx_handler_sync::perform }
};
