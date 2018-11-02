#include "sync_handler.h"

#include <memory>

base_sync_handler::base_sync_handler(const std::string &host, http_session_ptr session) 
    : base_handler(session)
{}

void base_sync_handler::execute() {
    BGN_TRY
    {
        executeImpl();
    }
    END_TRY
}
