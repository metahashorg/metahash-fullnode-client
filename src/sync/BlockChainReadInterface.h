#ifndef BLOCKCHAIN_READ_INTERFACE_H_
#define BLOCKCHAIN_READ_INTERFACE_H_

#include "OopUtils.h"

namespace torrent_node_lib {

struct BlockHeader;

class BlockChainReadInterface : public common::no_copyable, common::no_moveable {
public:
    
    virtual BlockHeader getBlock(const std::vector<unsigned char> &hash) const = 0;
    
    virtual BlockHeader getBlock(const std::string &hash) const = 0;
    
    virtual BlockHeader getBlock(size_t blockNumber) const = 0;
    
    virtual BlockHeader getLastBlock() const = 0;
    
    virtual size_t countBlocks() const = 0;
    
    virtual ~BlockChainReadInterface() = default;

};

}

#endif // BLOCKCHAIN_INTERFACE_H_
