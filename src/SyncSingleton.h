#ifndef SYNC_SINGLETON_H_
#define SYNC_SINGLETON_H_

#include <memory>

#include "sync/synchronize_blockchain.h"

std::unique_ptr<torrent_node_lib::Sync>& syncSingleton();

#endif // SYNC_SINGLETON_H_
