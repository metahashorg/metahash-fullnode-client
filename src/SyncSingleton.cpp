#include "SyncSingleton.h"

std::unique_ptr<Sync>& syncSingleton() {
    static std::unique_ptr<Sync> sync = nullptr;
    return sync;
}
