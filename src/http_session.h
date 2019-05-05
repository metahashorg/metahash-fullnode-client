#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace   asio    = boost::asio;
namespace   ip      = boost::asio::ip;
using       tcp     = boost::asio::ip::tcp;
namespace   beast   = boost::beast;
namespace   http    = boost::beast::http;

class http_session: public std::enable_shared_from_this<http_session>
{
public:
    http_session() = delete;
    http_session(const http_session& session) = delete;
    http_session& operator=(http_session session) = delete;

    http_session(tcp::socket&& socket);
    ~http_session();

    void run();

    asio::io_context& get_io_context();

    void send_json(const std::string& data);

protected:
    void process_request();
    void process_post_request();
    void process_get_request();

    void send_response(http::response<http::string_body>& response);
    void send_bad_request(const char* error);

    void close();
    bool keep_alive();

private:
    tcp::socket                      m_socket;
    beast::flat_buffer               m_buf{ 8192 };
    http::request<http::string_body> m_req;
    unsigned                         m_http_ver;
    bool                             m_http_keep_alive;
};

#endif // HTTP_SESSION_H
