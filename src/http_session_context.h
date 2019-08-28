#ifndef __HTTP_SESSION_CONTEXT_H__
#define __HTTP_SESSION_CONTEXT_H__

#include <memory>

namespace boost {
namespace asio {
    class io_context;
}
}

class session_context: public std::enable_shared_from_this<session_context>
{
public:
    explicit session_context() {}
    //session_context(const session_context& src) = delete;
    //session_context& operator=(session_context src) = delete;
    virtual ~session_context() {}

    template <typename T>
    std::shared_ptr<T> shared_from(T*) {
        return std::static_pointer_cast<T>(shared_from_this());
    }

    virtual void send_json(const char* data, size_t size) = 0;
    virtual boost::asio::io_context& get_io_context() = 0;
    virtual const std::string& get_remote_ep() const = 0;
};

#endif // __HTTP_SESSION_CONTEXT_H__
