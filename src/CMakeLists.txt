set(BOOST_LIBS "" CACHE STRING "Path to boost lib")
if ("${BOOST_LIBS}" STREQUAL "")
    find_package(Boost 1.67)
    set(BOOST_LIBS_INCLUDE ${Boost_INCLUDE_DIRS})
    set(BOOST_LIBS_LIBS ${Boost_LIBRARY_DIRS})
else()
    set(BOOST_LIBS_INCLUDE ${BOOST_LIBS}/include)
    set(BOOST_LIBS_LIBS ${BOOST_LIBS}/lib)
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    exception/except.cpp
    settings/settings.cpp
    http_server.cpp
    http_json_rpc_request.cpp
    json_rpc.cpp
    http_session.cpp
    log/log.cpp
    task_handlers/utils.cpp
    task_handlers/task_handlers.h
    task_handlers/base_handler.cpp
    task_handlers/network_handler.cpp
    task_handlers/create_tx_base_handler.cpp
    task_handlers/create_tx_handler.cpp
    task_handlers/send_tx_handler.cpp
    task_handlers/generate_handler.cpp
    task_handlers/fetch_balance_handler.cpp
    task_handlers/fetch_history_handler.cpp
    task_handlers/get_tx_handler.cpp
    task_handlers/get_dump_block_by_hash_handler.cpp
    task_handlers/get_dump_block_by_number_handler.cpp
    task_handlers/get_blocks_handler.cpp
    task_handlers/get_block_by_number_handler.cpp
    task_handlers/get_block_by_hash_handler.cpp
    task_handlers/get_last_txs_handler.cpp
    task_handlers/get_count_blocks_handler.cpp
    wallet_storage/wallet_storage.cpp
    cpplib_open_ssl_decor/crypto.cpp
    
    SyncSingleton.cpp
    
    task_handlers/task_handlers.cpp
    
    task_handlers/sync_handler.cpp
    task_handlers/fetch_balance_handler_sync.cpp
    task_handlers/fetch_history_handler_sync.cpp
    task_handlers/send_tx_handler_sync.cpp
    
    generate_json.cpp
    )

include_directories( SYSTEM
    #${Boost_LIBRARY_DIRS} /
    ${OPENSSL_INCLUDE_DIRS} 
    ${BOOST_LIBS_INCLUDE}
    ../3rdParty/log4cpp/include/
    )
include_directories(
    common/
    sync/
    ./
    )


find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

#find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
target_link_libraries(${PROJECT_NAME} "${BOOST_LIBS_LIBS}/libboost_filesystem.a" "${BOOST_LIBS_LIBS}/libboost_program_options.a" "${BOOST_LIBS_LIBS}/libboost_system.a")

link_directories(${PROJECT_NAME} ${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

find_library(COMMON_LIB common HINTS common/)
find_library(LOG4CPP log4cpp HINTS ../3rdParty/log4cpp/)
find_library(SYNC torrent_node_lib HINTS sync/)
find_library(LEVELDB NAMES libleveldb.a HINTS sync/)
find_library(LEVELDB_SNAPPY NAMES libsnappy.a HINTS sync/)
find_library(LEVELDB_MEMENV NAMES libmemenv.a HINTS sync/)
target_link_libraries(${PROJECT_NAME} ${SYNC} ${COMMON_LIB} ${LEVELDB} ${LEVELDB_SNAPPY} ${LEVELDB_MEMENV} ${LOG4CPP})
target_link_libraries(${PROJECT_NAME} stdc++fs.a curl)

find_package(OpenSSL REQUIRED)
#target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/usr/local/lib/libcrypto.so /usr/local/lib/libssl.so")
set (CMAKE_CXX_FLAGS "-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -pthread -Ofast -Wall -Wextra -Werror -Wno-unused-parameter")
#set (CMAKE_CXX_FLAGS "-g -rdynamic")
