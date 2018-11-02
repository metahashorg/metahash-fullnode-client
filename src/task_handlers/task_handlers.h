#pragma once

#include <memory>
#include <map>

#include "network_handler.h"
#include "create_tx_handler.h"
#include "send_tx_handler.h"
#include "sync_handler.h"

using handler_func = handler_result(*)(http_session_ptr session, const std::string& params);

using UseLocalDatabase = bool;
extern const std::map<std::pair<std::string, UseLocalDatabase>, handler_func> map_handlers;
