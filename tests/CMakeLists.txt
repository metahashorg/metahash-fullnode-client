#add_subdirectory(handlers)

cmake_minimum_required(VERSION 3.10.0)

include_directories(SYSTEM
    ./
    ../src/
    ../src/common/
    ../src/sync/
    ../3rdParty/
    ../3rdParty/ssl/include/
    ../3rdParty/log4cpp/include
)

message("Object files directory: " ${OBJECT_FILES_DIR})

set(TEST_NAME task_handlers_test)

add_executable(${TEST_NAME}
#    ${OBJECT_FILES_DIR}/log/log.cpp.o
#    ${OBJECT_FILES_DIR}/exception/except.cpp.o
#    ${OBJECT_FILES_DIR}/common/string_utils.cpp.o
#    ${OBJECT_FILES_DIR}/common/filesystem_utils.cpp.o
#    ${OBJECT_FILES_DIR}/json_rpc.cpp.o
#    ${OBJECT_FILES_DIR}/http_json_rpc_request.cpp.o
#    ${OBJECT_FILES_DIR}/settings/settings.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/utils.cpp.o
#    ${OBJECT_FILES_DIR}/wallet_storage/wallet_storage.cpp.o
#    ${OBJECT_FILES_DIR}/data_storage/data_address.cpp.o
#    ${OBJECT_FILES_DIR}/data_storage/data_storage.cpp.o
#    ${OBJECT_FILES_DIR}/data_storage/data_updater.cpp.o
#    ${OBJECT_FILES_DIR}/http_session.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/base_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/network_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/generate_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/create_tx_base_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/create_tx_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/send_tx_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/fetch_balance_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/fetch_history_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/add_addresses_to_batch.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/del_addresses_to_batch.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/get_addresses_to_batch.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/batch_balance.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/batch_history.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/create_tx_token_base_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/create_tx_token_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/send_tx_token_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/add_addresses_to_batch_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/del_addresses_to_batch_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/get_addresses_to_batch_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/batch_balance_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/batch_history_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/fetch_balance_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/fetch_history_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/get_transaction_params.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/get_transaction_params_tkn.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/add_to_tracking_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/del_from_tracking_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/get_tracking_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/status_handler.cpp.o
#    ${OBJECT_FILES_DIR}/task_handlers/task_handlers.cpp.o

#    ${OBJECT_FILES_DIR}/ethtx/crossguid/Guid.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/scrypt/crypto_scrypt-nosse.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/scrypt/crypto_scrypt_saltgen.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/scrypt/sha256.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/cert.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/cert2.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/ethtx.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/rlp.cpp.o
#    ${OBJECT_FILES_DIR}/ethtx/utils2.cpp.o
#    ${OBJECT_FILES_DIR}/eth_wallet/EthWallet.cpp.o
#    ${OBJECT_FILES_DIR}/eth_wallet/FileSystem.cpp.o
#    ${OBJECT_FILES_DIR}/eth_wallet/TypedException.cpp.o
#    ${OBJECT_FILES_DIR}/eth_wallet/jsonUtils.cpp.o

    task_handlers_test.cpp
    )

set(LIBS_FOR_TEST
    stdc++fs.a
    uuid
    ssl
    crypto
    cryptopp
    dl
    rt
)

link_directories(${TEST_NAME} ${Boost_LIBRARY_DIRS})
target_link_libraries(${TEST_NAME} ${Boost_LIBRARIES})

target_link_libraries(${TEST_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TEST_NAME} ${BOOST1} ${BOOST2} ${BOOST3})
link_directories(${TEST_NAME} ${Boost_LIBRARY_DIRS})
target_link_libraries(${TEST_NAME} ${Boost_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})

target_link_libraries(${TEST_NAME} ${LIB_CRYPTO} ${LIB_SSL} ${LIB_LEVELDB} ${LIB_P7} ${LIB_GMP} ${LIB_GMPXX} ${SECP} ${LIBS_FOR_TEST})

set (CMAKE_CXX_FLAGS "-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -pthread -Ofast -Wall -Wextra -Werror -Wfloat-equal")

target_compile_options(${TEST_NAME} PRIVATE -no-pie -D_GLIBCXX_USE_CXX11_ABI=0)
set_target_properties(${TEST_NAME} PROPERTIES LINK_FLAGS "-no-pie -D_GLIBCXX_USE_CXX11_ABI=0")
