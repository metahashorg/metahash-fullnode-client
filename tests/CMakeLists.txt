#add_subdirectory(handlers)

cmake_minimum_required(VERSION 3.8.0)

include_directories(SYSTEM
    ./
    ../src/
    ../src/common/
    ../src/sync/
    ../3rdParty/
    ../3rdParty/ssl/include/
    ../3rdParty/log4cpp/include
#    ${BOOST_LIBS_INCLUDE}
)

message("Object files directory: " ${OBJECT_FILES_DIR})

set(TEST_NAME task_handlers_test)

add_executable(${TEST_NAME}
    ${OBJECT_FILES_DIR}/exception/except.cpp.o
    ${OBJECT_FILES_DIR}/common/string_utils.cpp.o
    ${OBJECT_FILES_DIR}/common/filesystem_utils.cpp.o
    ${OBJECT_FILES_DIR}/json_rpc.cpp.o
    ${OBJECT_FILES_DIR}/http_json_rpc_request.cpp.o
    ${OBJECT_FILES_DIR}/settings/settings.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/utils.cpp.o
    ${OBJECT_FILES_DIR}/wallet_storage/wallet_storage.cpp.o
    ${OBJECT_FILES_DIR}/http_session.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/base_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/network_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/generate_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/create_tx_base_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/create_tx_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/send_tx_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/fetch_balance_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/fetch_history_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_tx_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_dump_block_by_hash_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_dump_block_by_number_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_blocks_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_block_by_number_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_block_by_hash_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_last_txs_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_count_blocks_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/status_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/task_handlers.cpp.o

    ${OBJECT_FILES_DIR}/cpplib_open_ssl_decor/crypto.cpp.o

    ${OBJECT_FILES_DIR}/SyncSingleton.cpp.o

    ${OBJECT_FILES_DIR}/__/GitSHA1.cpp.o

    ${OBJECT_FILES_DIR}/task_handlers/sync_handler.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/fetch_balance_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/fetch_history_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/send_tx_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_count_blocks_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_block_by_hash_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_block_by_number_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_tx_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_dump_block_by_hash_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/task_handlers/get_dump_block_by_number_handler_sync.cpp.o
    ${OBJECT_FILES_DIR}/generate_json.cpp.o

    task_handlers_test.cpp
    )

find_package(Threads)
target_link_libraries(${TEST_NAME} ${CMAKE_THREAD_LIBS_INIT})

#find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
target_link_libraries(${TEST_NAME} ${BOOST_LIBRS})

target_link_libraries(${TEST_NAME} ${BOOST1} ${BOOST2} ${BOOST3})

find_library(COMMON_LIB common HINTS ../srs/common/)
find_library(LOG4CPP log4cpp HINTS ../3rdParty/log4cpp/)
find_library(SYNC torrent_node_lib HINTS ../src/sync/)
find_library(LEVELDB NAMES libleveldb.a HINTS ../src/sync/)
find_library(LEVELDB_SNAPPY NAMES libsnappy.a HINTS ../src/sync/)
find_library(LEVELDB_MEMENV NAMES libmemenv.a HINTS ../src/sync/)
target_link_libraries(${TEST_NAME} ${SYNC} ${COMMON_LIB} ${LEVELDB} ${LEVELDB_SNAPPY} ${LEVELDB_MEMENV} ${LOG4CPP})
target_link_libraries(${TEST_NAME} stdc++fs.a curl resolv)

find_library(LIB_CRYPTO NAMES libcrypto.so HINTS ../3rdParty/ssl/lib/)
find_library(LIB_SSL NAMES libssl.so HINTS ../3rdParty/ssl/lib/)

find_package(OpenSSL REQUIRED)
#target_link_libraries(${TEST_NAME} ${OPENSSL_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(${TEST_NAME} ${LIB_CRYPTO} ${LIB_SSL})
set (CMAKE_CXX_FLAGS "-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -pthread -Ofast -Wall -Wextra -Werror -Wno-unused-parameter")
#set (CMAKE_CXX_FLAGS "-g -rdynamic")
target_compile_options(${TEST_NAME} PRIVATE -no-pie -D_GLIBCXX_USE_CXX11_ABI=0)
set_target_properties(${TEST_NAME} PROPERTIES LINK_FLAGS "-no-pie -D_GLIBCXX_USE_CXX11_ABI=0")
