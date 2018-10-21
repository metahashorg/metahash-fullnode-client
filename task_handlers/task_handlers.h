#pragma once

#include <memory>
#include <map>
#include "network_handler.h"
#include "create_tx_handler.h"
#include "send_tx_handler.h"
#include "../log/log.h"

DECL_BASE_HANDLER(generate_handler)
DECL_NETWORK_HANDLER(get_count_blocks_handler)
DECL_NETWORK_HANDLER(get_last_txs_handler)
DECL_NETWORK_HANDLER(get_block_by_hash_handler)
DECL_NETWORK_HANDLER(get_block_by_number_handler)
DECL_NETWORK_HANDLER(get_blocks_handler)
DECL_NETWORK_HANDLER(get_dump_block_by_number)
DECL_NETWORK_HANDLER(get_dump_block_by_hash)
DECL_NETWORK_HANDLER(get_tx_handler)
DECL_NETWORK_HANDLER(fetch_balance_handler)
DECL_NETWORK_HANDLER(fetch_history_handler)

// MAP OF HANDLERS

using handler_func = handler_result(*)(http_session_ptr session, const std::string& params);

static const std::map<std::string, handler_func> map_handlers = {
    { "generate",					generate_handler::perform },
    { "create-tx",					create_tx_handler::perform },
    { "send-tx",					send_tx_handler::perform },
    { "get-count-blocks",			get_count_blocks_handler::perform },
    { "get-last-txs",				get_last_txs_handler::perform },
    { "get-block-by-hash",			get_block_by_hash_handler::perform },
    { "get-block-by-number",		get_block_by_number_handler::perform },
    { "get-blocks",					get_blocks_handler::perform },
    { "get-dump-block-by-number",	get_dump_block_by_number::perform },
    { "get-dump-block-by-hash",		get_dump_block_by_hash::perform },
    { "get-tx",						get_tx_handler::perform },
    { "fetch-balance",				fetch_balance_handler::perform },
    { "fetch-history",				fetch_history_handler::perform }
};
