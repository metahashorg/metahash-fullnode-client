#ifndef __TASK_HANDLERS_H__
#define __TASK_HANDLERS_H__

#include <memory>
#include <map>

#include "http_session_context_ptr.h"

struct handler_result;

using handler_func = handler_result(*)(session_context_ptr ctx, const std::string& params);

using UseLocalDatabase = bool;
extern const std::map<std::pair<std::string, UseLocalDatabase>, handler_func> post_handlers;

extern const std::map<std::string_view, handler_func> get_handlers;

#endif // __TASK_HANDLERS_H__
