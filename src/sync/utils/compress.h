#ifndef COMPRESS_H_ 
#define COMPRESS_H_

#include <string>

namespace torrent_node_lib {
    
std::string compress(const std::string &value);

std::string decompress(const std::string &value);
    
} // namespace torrent_node_lib

#endif // COMPRESS_H_
