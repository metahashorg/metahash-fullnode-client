#pragma once

#include <memory>
#include <map>

#include "http_session_ptr.h"

struct handler_result;

using handler_func = handler_result(*)(http_session_ptr session, const std::string& params);

using UseLocalDatabase = bool;
extern const std::map<std::pair<std::string, UseLocalDatabase>, handler_func> map_handlers;
