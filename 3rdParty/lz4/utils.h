/*
 * Copyright (c) 2018 - 2019, MetaHash, Oleg Romanenko (oleg@romanenko.ro)
 *
 */

#pragma once

#include <cstring>
#include <endian.h>
#include <sniper/lz4/lz4.h>
#include <sniper/std/string.h>

namespace sniper::lz4 {

inline bool compress_raw_block(string_view src, string& dst)
{
    if (src.empty())
        return false;

    int bound_size = LZ4_compressBound(src.size());
    if (!bound_size)
        return false;

    if (dst.size() < (uint32_t)bound_size)
        dst.resize(bound_size);

    int lz4_size = LZ4_compress_default(src.data(), dst.data(), src.size(), dst.size());
    if (lz4_size) {
        if (lz4_size != bound_size)
            dst.resize(lz4_size);
        return true;
    }

    return false;
}

inline bool compress_uint32_block(string_view src, string& dst)
{
    if (src.empty())
        return false;

    int bound_size = LZ4_compressBound(src.size());
    if (!bound_size)
        return false;

    bound_size += sizeof(uint32_t);


    if (dst.size() < (uint32_t)bound_size)
        dst.resize(bound_size);

    int lz4_size =
        LZ4_compress_default(src.data(), dst.data() + sizeof(uint32_t), src.size(), dst.size() - sizeof(uint32_t));
    if (lz4_size) {
        if ((lz4_size + (int)sizeof(uint32_t)) != bound_size)
            dst.resize(lz4_size + sizeof(uint32_t));

        uint32_t* header = (uint32_t*)dst.data();
        *header = htole32(src.size());

        return true;
    }

    return false;
}

inline bool decompress_raw_block(string_view src, string& dst)
{
    if (src.empty())
        return false;

    int size = LZ4_decompress_safe(src.data(), dst.data(), src.size(), dst.size());
    if (size < 0)
        return false;

    if (dst.size() != (uint32_t)size)
        dst.resize(size);

    return true;
}

inline bool decompress_uint32_block(string_view src, string& dst, uint32_t max_size = 128 * 1024)
{
    if (src.size() <= 4)
        return false;

    uint32_t orig_size = 0;
    memcpy((char*)&orig_size, src.data(), sizeof(uint32_t));
    orig_size = le32toh(orig_size);

    if (orig_size > max_size)
        return false;

    if (dst.size() < orig_size)
        dst.resize(orig_size);

    int size =
        LZ4_decompress_safe(src.data() + sizeof(uint32_t), dst.data(), src.size() - sizeof(uint32_t), dst.size());
    if (size < 0)
        return false;

    if (dst.size() != (uint32_t)size)
        dst.resize(size);

    return true;
}

} // namespace sniper::lz4
