#include "sync_handler.h"

#include <memory>

base_sync_handler::base_sync_handler(session_context_ptr ctx)
    : base_handler(ctx)
{}

void base_sync_handler::execute() {
    BGN_TRY
    {
        executeImpl();
    }
    END_TRY
}
