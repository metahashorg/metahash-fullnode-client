set (BOOST_LIBS "/home/user/boost_1_68_0/local")

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
    )

set(CMAKE_CXX_FLAGS "-std=c++14")

include_directories(
    #${Boost_LIBRARY_DIRS} /
    ${OPENSSL_INCLUDE_DIRS} 
    ${BOOST_LIBS}/include/
    )

find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

#find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
target_link_libraries(${PROJECT_NAME} "${BOOST_LIBS}/lib/libboost_filesystem.a" "${BOOST_LIBS}/lib/libboost_program_options.a" "${BOOST_LIBS}/lib/libboost_system.a")

link_directories(${PROJECT_NAME} ${Boost_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

find_library(COMMON_LIB common HINTS common/)
target_link_libraries(${PROJECT_NAME} ${COMMON_LIB} log4cpp.a)

find_package(OpenSSL REQUIRED)
#target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
include_directories(${OPENSSL_INCLUDE_DIR})
set (CMAKE_CXX_FLAGS "/usr/local/lib/libcrypto.so /usr/local/lib/libssl.so")
