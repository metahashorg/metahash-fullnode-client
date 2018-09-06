cmake_minimum_required(VERSION 2.8)

project(metahash_proxy_service)

set (MH_CRYPT_PATH "cpplib_open_ssl_decor")
set (MH_CRYPT_OBJS "${MH_CRYPT_PATH}/crypto.o")

add_executable(${PROJECT_NAME} /
    "main.cpp" /
    "settings/settings.cpp" /
    "http_server.cpp" /
    "http_json_rpc_request.cpp" /
    "json_rpc.cpp" /
    "http_session.cpp" /
    "task_handlers/utils.cpp" /
    "task_handlers/task_handlers.h" /
    "task_handlers/base_handler.cpp" /
    "task_handlers/network_handler.cpp" /
    "task_handlers/create_tx_handler.cpp" /
    "task_handlers/send_tx_handler.cpp" /
    "task_handlers/generate_handler.cpp" /
    "task_handlers/fetch_balance_handler.cpp" /
    "task_handlers/fetch_history_handler.cpp" /
    "task_handlers/get_tx_handler.cpp" /
    "task_handlers/get_dump_block_by_hash_handler.cpp" /
    "task_handlers/get_dump_block_by_number_handler.cpp" /
    "task_handlers/get_blocks_handler.cpp" /
    "task_handlers/get_block_by_number_handler.cpp" /
    "task_handlers/get_block_by_hash_handler.cpp" /
    "task_handlers/get_last_txs_handler.cpp" /
    "task_handlers/get_count_blocks_handler.cpp" /
    "wallet_storage/wallet_storage.cpp" /
    ${MH_CRYPT_OBJS})

set(CMAKE_CXX_FLAGS "-std=c++14 -lssl -lcrypto -lboost_program_options -lboost_system")

include_directories(
    ${Boost_LIBRARY_DIRS} /
    ${OPENSSL_INCLUDE_DIRS} /
    ${MH_CRYPT_PATH})


find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
link_directories(${PROJECT_NAME} ${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
