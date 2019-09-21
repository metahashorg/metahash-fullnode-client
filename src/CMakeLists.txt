list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_local_changes(GIT_LOCAL_CHANGES)
git_get_commit_datetime(GIT_DATE)

message(STATUS "Git SHA1: " ${GIT_SHA1})
message(STATUS "Git DateTime: " ${GIT_DATE})

configure_file("${CMAKE_CURRENT_LIST_DIR}/cmake_modules/GitSHA1.cpp.in" "${CMAKE_BINARY_DIR}/GitSHA1.cpp" @ONLY)

# for using in other cmake's
set(OBJECT_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}.dir" PARENT_SCOPE)

set(BOOST_LIBS_INCLUDE ../3rdParty/boost/include)
set(BOOST_LIBS_LIBS ../3rdParty/boost/lib)
find_library(BOOST1 NAMES libboost_filesystem.a HINTS ${BOOST_LIBS_LIBS})
find_library(BOOST2 NAMES libboost_program_options.a HINTS ${BOOST_LIBS_LIBS})
find_library(BOOST3 NAMES libboost_system.a HINTS ${BOOST_LIBS_LIBS})

function( add_resource input )
    string( MAKE_C_IDENTIFIER ${input} input_identifier )
    set( output ${input_identifier}.o )
    set(output_dir "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}.dir/${output}")
    add_custom_command(
            OUTPUT ${output_dir}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ld -r -b binary -o ${output_dir} ${input}
            DEPENDS ${input}
            COMMENT "Link resource file: ${output}"
    )
    set( RESOURCES ${RESOURCES} ${output_dir} PARENT_SCOPE)
endfunction()

add_resource("jsonrpc/schema/schema_request.json")
add_resource("jsonrpc/schema/schema_methods.json")

add_executable(${PROJECT_NAME}
    main.cpp
    common/string_utils.cpp
    common/filesystem_utils.cpp
    exception/except.cpp
    settings/settings.cpp
    cache/compress.cpp
    cache/blocks_cache.cpp
    cache/history_cache.cpp
    connection_pool.cpp
    http_server.cpp
    http_json_rpc_request.cpp
    jsonrpc/json_rpc_schema.cpp
    jsonrpc/json_rpc.cpp
    http_session_context.cpp
    json_batch_request.cpp
    http_session.cpp
    nslookup.cpp
    
    task_handlers/utils.cpp
    task_handlers/task_handlers.h
    task_handlers/base_handler.cpp
    task_handlers/network_handler.cpp
    task_handlers/create_tx_base_handler.cpp
    task_handlers/create_tx_handler.cpp
    task_handlers/send_tx_handler.cpp
    task_handlers/generate_handler.cpp
    task_handlers/addr_validate_handler.cpp
    task_handlers/fetch_balance_handler.cpp
    task_handlers/fetch_balances_handler.cpp
    task_handlers/fetch_history_handler.cpp
    task_handlers/fetch_history_filter_handler.cpp
    task_handlers/get_tx_handler.cpp
    task_handlers/get_dump_block_by_hash_handler.cpp
    task_handlers/get_dump_block_by_number_handler.cpp
    task_handlers/get_blocks_handler.cpp
    task_handlers/get_block_by_number_handler.cpp
    task_handlers/get_block_by_hash_handler.cpp
    task_handlers/get_last_txs_handler.cpp
    task_handlers/get_count_blocks_handler.cpp
    task_handlers/status_handler.cpp
    task_handlers/fetch_transaction_handler.cpp
    
    wallet_storage/wallet_storage.cpp
    cpplib_open_ssl_decor/crypto.cpp
    
    SyncSingleton.cpp
    
    task_handlers/task_handlers.cpp
    
    task_handlers/sync_handler.cpp
    task_handlers/fetch_balance_handler_sync.cpp
    task_handlers/fetch_history_handler_sync.cpp
    task_handlers/send_tx_handler_sync.cpp
    task_handlers/get_count_blocks_handler_sync.cpp
    task_handlers/get_block_by_hash_handler_sync.cpp
    task_handlers/get_block_by_number_handler_sync.cpp
    task_handlers/get_tx_handler_sync.cpp
    task_handlers/get_dump_block_by_hash_handler_sync.cpp
    task_handlers/get_dump_block_by_number_handler_sync.cpp
    
    ${CMAKE_BINARY_DIR}/GitSHA1.cpp
    
    generate_json.cpp

    ${RESOURCES}
    )

include_directories( SYSTEM
    ${BOOST_LIBS_INCLUDE}
    ../3rdParty/ssl/include/
    ../3rdParty/log4cpp/include/
    ../3rdParty/leveldb/include/
    ../3rdParty/lz4/include/
    ../3rdParty/
    common/
    sync/
    jsonrpc/
    jsonrpc/schema/
    ./
)

find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

#find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
target_link_libraries(${PROJECT_NAME} ${BOOST_LIBRS})
target_link_libraries(${PROJECT_NAME} ${BOOST1} ${BOOST2} ${BOOST3})

find_library(COMMON_LIB common HINTS common/)
find_library(LOG4CPP log4cpp HINTS ../3rdParty/log4cpp/)
find_library(SYNC torrent_node_lib HINTS sync/)
find_library(LEVELDB NAMES libleveldb.a HINTS sync/)
find_library(LEVELDB_SNAPPY NAMES libsnappy.a HINTS sync/)
find_library(LEVELDB_MEMENV NAMES libmemenv.a HINTS sync/)
find_library(LZ4 NAMES lz4 HINTS ../3rdParty/lz4/lib/)
target_link_libraries(${PROJECT_NAME} ${SYNC} ${COMMON_LIB} ${LEVELDB} ${LEVELDB_SNAPPY} ${LEVELDB_MEMENV} ${LOG4CPP} ${LZ4})
target_link_libraries(${PROJECT_NAME} stdc++fs.a curl resolv)

find_library(LIB_CRYPTO NAMES libcrypto.so HINTS ../3rdParty/ssl/lib/)
find_library(LIB_SSL NAMES libssl.so HINTS ../3rdParty/ssl/lib/)
find_library(SECP_LIB NAMES libsecp256k1.a HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/secp256k1/lib/)

#find_package(OpenSSL REQUIRED)
#target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
#include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LIB_CRYPTO} ${LIB_SSL} ${SECP_LIB} gmp)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set (CMAKE_CXX_FLAGS "-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -pthread -Ofast -Wall -Wextra -Werror -Wno-unused-parameter")
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")
    set (CMAKE_CXX_FLAGS "-Werror -pthread")
endif()

#set (CMAKE_CXX_FLAGS "-g -rdynamic")
target_compile_options(${PROJECT_NAME} PRIVATE -no-pie -D_GLIBCXX_USE_CXX11_ABI=0)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-no-pie -D_GLIBCXX_USE_CXX11_ABI=0")
