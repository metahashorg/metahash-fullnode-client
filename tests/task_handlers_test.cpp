#ifndef TASK_HANDLERS_TEST_H_
#define TASK_HANDLERS_TEST_H_

#define BOOST_TEST_MODULE task handlers
#include <boost/test/included/unit_test.hpp>
#include "task_handlers/task_handlers.h"
#include "task_handlers/generate_handler.h"
#include "task_handlers/fetch_balance_handler.h"
#include "task_handlers/fetch_history_handler.h"
#include "task_handlers/create_tx_handler.h"
#include "task_handlers/send_tx_handler.h"
#include "task_handlers/status_handler.h"
#include "task_handlers/get_tx_handler.h"
#include "task_handlers/get_dump_block_by_hash.h"
#include "task_handlers/get_dump_block_by_number.h"
#include "task_handlers/get_blocks_handler.h"
#include "task_handlers/get_block_by_number_handler.h"
#include "task_handlers/get_block_by_hash_handler.h"
#include "task_handlers/get_last_txs_handler.h"
#include "task_handlers/get_count_blocks_handler.h"


namespace {

template<class T>
bool test(const std::string& name, const std::string& input, const std::string& output, bool check_map = true)
{
    T handler(nullptr);
    if (check_map && post_handlers.find(std::make_pair(name, false)) == post_handlers.end()) {
        BOOST_FAIL("\nHandler: " << name << "\n"
                  << "Error:   does not found in handlers map");
        return false;
    }
    if (!handler.prepare(input)) {
        BOOST_FAIL("\nHandler: " << name << "\n"
                  << "Error:   Prepare failed\n"
                  << "Input:   " << input);
        return false;
    }
    handler_result res = handler.result();
    if (!res) {
        BOOST_FAIL("\nHandler: " << name << "\n"
                  << "Error:   Result failed");
        return false;
    }
    if (res.message != output) {
        BOOST_FAIL("\nHandler: " << name << "\n"
                  << "Error:   Not equal\n"
                  << "Result:  " << res.message << "\n"
                  << "Expect:  " << output);
        return false;
    }
    return true;
};

};

BOOST_AUTO_TEST_CASE(handlers_test)
{
}
/*
    test<generate_handler>("generate",
        "{\"id\":123, \"method\":\"generate\", \"params\":{\"password\":\"aqws123Za\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123}");

    test<fetch_balance_handler>("fetch-balance",
        "{\"id\":123, \"method\":\"fetch-balance\", \"params\":{\"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"address.balance\",\"token\":\"\",\"params\":[{\"currency\":-1,\"address\":[\"0x1234567890\"]}]}");

    test<fetch_history_handler>("fetch-history",
        "{\"id\":123, \"method\":\"fetch-history\", \"params\":{\"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"address.transaction\",\"token\":\"\",\"params\":[{\"currency\":-1,\"address\":\"0x1234567890\"}]}");

    test<add_addresses_to_batch>("add-addresses-to-batch",
        "{\"id\":123, \"method\":\"add-addresses-to-batch\", \"params\":{\"group\":\"group_name\", \"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"batch.addresses.add\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"address\":\"0x1234567890\"}]}");

    test<del_addresses_to_batch>("del-addresses-to-batch",
        "{\"id\":123, \"method\":\"del-addresses-to-batch\", \"params\":{\"group\":\"group_name\", \"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"batch.addresses.remove\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"address\":\"0x1234567890\"}]}");

    test<get_addresses_to_batch>("get-addresses-to-batch",
        "{\"id\":123, \"method\":\"get-addresses-to-batch\", \"params\":{\"group\":\"group_name\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"batch.addresses\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\"}]}");

    test<batch_balance>("batch-balance",
        "{\"id\":123, \"method\":\"batch-balance\", \"params\":{\"group\":\"group_name\", \"block\":1234567890}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"batch.last.balance\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"block\":1234567890}]}");

    test<batch_history>("batch-history",
        "{\"id\":123, \"method\":\"batch-history\", \"params\":{\"group\":\"group_name\", \"block\":1234567890}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"batch.last.history\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"block\":1234567890}]}");

    test<add_addresses_to_batch_tkn>("add-addresses-to-batch-tkn",
        "{\"id\":123, \"method\":\"add-addresses-to-batch-tkn\", \"params\":{\"group\":\"group_name\", \"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"tkn.batch.addresses.add\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"address\":\"0x1234567890\"}]}");

    test<del_addresses_to_batch_tkn>("del-addresses-to-batch-tkn",
        "{\"id\":123, \"method\":\"del-addresses-to-batch-tkn\", \"params\":{\"group\":\"group_name\", \"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"tkn.batch.addresses.remove\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"address\":\"0x1234567890\"}]}");

    test<get_addresses_to_batch_tkn>("get-addresses-to-batch-tkn",
        "{\"id\":123, \"method\":\"get-addresses-to-batch-tkn\", \"params\":{\"group\":\"group_name\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"tkn.batch.addresses\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\"}]}");

    test<batch_balance_tkn>("batch-balance-tkn",
        "{\"id\":123, \"method\":\"batch-balance-tkn\", \"params\":{\"group\":\"group_name\", \"block\":1234567890}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"tkn.batch.last.balance\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"block\":1234567890}]}");

    test<batch_history_tkn>("batch-history-tkn",
        "{\"id\":123, \"method\":\"batch-history-tkn\", \"params\":{\"group\":\"group_name\", \"block\":1234567890}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"tkn.batch.last.history\",\"token\":\"\",\"params\":[{\"currency\":-1,\"group\":\"group_name\",\"block\":1234567890}]}");

    storage::addresses::group("group-123");
    settings::service::local_data = true;

    test<add_to_tracking_handler>("add-to-tracking",
        "{\"id\":123, \"method\":\"add-to-tracking\", \"params\":{\"address\":\"0x1234567890AbCdEf\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123}");

    test<get_tracking_handler>("get-tracking",
        "{\"id\":123, \"method\":\"get-tracking\"}",
        "{\"jsonrpc\":\"2.0\",\"id\":123}");

    settings::service::local_data = false;

    test<fetch_balance_tkn>("fetch-balance-tkn",
        "{\"id\":123, \"method\":\"fetch-balance-tkn\", \"params\":{\"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"address.tkn.balance\",\"token\":\"\",\"params\":[{\"currency\":-1,\"address\":\"0x1234567890\"}]}");

    test<fetch_history_tkn>("fetch-history-tkn",
        "{\"id\":123, \"method\":\"fetch-history-tkn\", \"params\":{\"address\":\"0x1234567890\",\"contract\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"address.tkn.transaction\",\"token\":\"\",\"params\":[{\"currency\":-1,\"address\":\"0x1234567890\",\"contract\":\"0x1234567890\"}]}");

    test<get_transaction_params>("get-transaction-params",
        "{\"id\":123, \"params\":{\"address\":\"0x1234567890\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"transaction.params\",\"token\":\"\",\"params\":[{\"currency\":-1,\"address\":\"0x1234567890\"}]}",
        false);

    test<get_transaction_params_tkn>("get-transaction-params-tkn",
        "{\"id\":123, \"params\":{\"address\":\"0x1234567890\",\"to_address\":\"0x0987654321\",\"value\":\"100000000000000000000\"}}",
        "{\"jsonrpc\":\"2.0\",\"id\":123,\"method\":\"tkn.transaction.params\",\"token\":\"\",\"params\":[{\"currency\":-1,\"address\":\"0x1234567890\",\"to_address\":\"0x0987654321\",\"value\":\"0x56bc75e2d63100000\"}]}",
        false);
}
*/
#endif // TASK_HANDLERS_TEST_H_
