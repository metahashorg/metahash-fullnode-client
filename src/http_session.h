#ifndef __HTTP_SESSION_H__
#define __HTTP_SESSION_H__

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "http_session_context.h"

namespace   asio    = boost::asio;
namespace   ip      = boost::asio::ip;
using       tcp     = boost::asio::ip::tcp;
namespace   beast   = boost::beast;
namespace   http    = boost::beast::http;

class json_rpc_reader;

class http_session: public session_context
{
public:
    http_session() = delete;
    http_session(const http_session& session) = delete;
    http_session& operator=(http_session session) = delete;

    http_session(tcp::socket&& socket);
    virtual ~http_session() override;

    void run();

    asio::io_context& get_io_context() override;

    void send_json(const char* data, size_t size) override;

protected:
    void process_request();
    void process_post_request();
    void process_single_request(const json_rpc_reader& reader);
    void process_batch_request(const json_rpc_reader& reader);
    void process_get_request();

    void send_response(http::response<http::string_body>& response);
    void send_bad_response(http::status status, const char* error = nullptr);

    void close();
    bool keep_alive();

    bool check_auth(const http::request<http::string_body>& req);

private:
    tcp::socket                      m_socket;
    beast::flat_buffer               m_buf{ 8192 };
    http::request<http::string_body> m_req;
    unsigned                         m_http_ver;
    bool                             m_http_keep_alive;
};

#endif // __HTTP_SESSION_H__
