cmake_minimum_required(VERSION 3.8)

set(LIB lz4)

include_directories(.)


set(LIB_HEADERS
    lz4.h
    lz4frame.h
    lz4frame_static.h
    lz4hc.h
    lz4opt.h
    utils.h
    )


set(LIB_SOURCES
    lz4.c
    lz4frame.c
    lz4hc.c
    )



#Library
add_library(_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_compile_options(_${LIB} PRIVATE -Wno-implicit-fallthrough)
