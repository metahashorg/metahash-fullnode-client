#ifndef HTTP_SESSION_PTR_H_
#define HTTP_SESSION_PTR_H_

#include <memory>

class http_session;

using http_session_ptr = std::shared_ptr<http_session>;

#endif // HTTP_SESSION_PTR_H_
