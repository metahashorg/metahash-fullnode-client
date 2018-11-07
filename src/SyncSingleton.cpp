#include "SyncSingleton.h"

std::unique_ptr<torrent_node_lib::Sync>& syncSingleton() {
    static std::unique_ptr<torrent_node_lib::Sync> sync = nullptr;
    return sync;
}
