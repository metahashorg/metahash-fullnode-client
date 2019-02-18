#ifndef LEVELDB_OPTIONS_H_
#define LEVELDB_OPTIONS_H_

#include <string>

namespace torrent_node_lib {

struct LevelDbOptions {
    size_t writeBufSizeMb;
    bool isBloomFilter;
    bool isChecks;
    std::string folderName;
    size_t lruCacheMb;
    
    bool isValid = false;
    
    LevelDbOptions() = default;
    
    LevelDbOptions(size_t writeBufSizeMb, bool isBloomFilter, bool isChecks, std::string_view folderName, size_t lruCacheMb)
        : writeBufSizeMb(writeBufSizeMb)
        , isBloomFilter(isBloomFilter)
        , isChecks(isChecks)
        , folderName(folderName)
        , lruCacheMb(lruCacheMb)
        , isValid(true)
    {}
};

}

#endif // LEVELDB_OPTIONS_H_
