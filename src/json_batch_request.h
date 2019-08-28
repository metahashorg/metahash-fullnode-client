#ifndef __JSON_BATCH_REQUEST_H__
#define __JSON_BATCH_REQUEST_H__

#define RAPIDJSON_HAS_STDSTRING 1

#include "http_session_context.h"
#include "http_session_context_ptr.h"
#include "rapidjson/document.h"
#include <mutex>

namespace boost {
namespace asio {
    class io_context;
}
}

class json_rpc_reader;

class batch_json_request: public session_context
{
public:
    batch_json_request(session_context_ptr ctx);
    virtual ~batch_json_request() override;

    boost::asio::io_context& get_io_context() override;
    void send_json(const char* data, size_t size) override;

    void process(const json_rpc_reader& reader);

protected:
    const std::string& get_remote_ep() const override;

private:
    session_context_ptr m_ctx;
    std::mutex          m_lock;
    rapidjson::Document m_result;
    unsigned int        m_size;
};

#endif // __JSON_BATCH_REQUEST_H__
