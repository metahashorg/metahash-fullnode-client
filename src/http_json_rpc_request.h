#pragma once

#include <string.h>

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "json_rpc.h"
#include "task_handlers/time_duration.h"
#include "task_handlers/utils.h"

namespace	asio    = boost::asio;
namespace	ssl     = boost::asio::ssl;
namespace	ip      = boost::asio::ip;
using		tcp     = boost::asio::ip::tcp;
namespace   beast   = boost::beast;
namespace	http    = boost::beast::http;

using http_json_rpc_execute_callback = std::function<void()>;

class http_json_rpc_request: public std::enable_shared_from_this<http_json_rpc_request>
{
public:
    http_json_rpc_request(const std::string& host, asio::io_context& execute_context);
    ~http_json_rpc_request();

    void set_path(const std::string& path);
    void set_body(const std::string& body);

    void execute();
    void execute_async(http_json_rpc_execute_callback callback);

    std::string get_result();

protected:
    void on_resolve(const boost::system::error_code& e, tcp::resolver::results_type eps);
    void on_connect(const boost::system::error_code& ec, const tcp::endpoint& ep);
    void on_connect2(const boost::system::error_code& e, std::vector<tcp::endpoint>::iterator i);
    void on_handshake(const boost::system::error_code& e);
    void on_write(const boost::system::error_code& e);
    void on_read(const boost::system::error_code& e);
    void on_request_timeout();
    void on_connect_timeout();

    bool error_handler(const boost::system::error_code& e);

    void perform_callback();

    bool verify_certificate(bool preverified, ssl::verify_context& ctx);

    inline bool is_ssl() const { return m_use_ssl; }

private:
    asio::io_context&                   m_io_ctx;
    tcp::socket                         m_socket;
    tcp::resolver                       m_resolver;
    utils::Timer                        m_timer;
    utils::Timer                        m_connect_timer;
    utils::time_duration                m_duration;
    http::request<http::dynamic_body>   m_req { http::verb::post, "/", 11 };
    http::response<http::string_body>   m_response;
    boost::beast::flat_buffer           m_buf { 8192 };
    json_rpc_writer                     m_result;
    http_json_rpc_execute_callback      m_callback;
    std::string                         m_host;
    ssl::context                        m_ssl_ctx;
    ssl::stream<tcp::socket>            m_ssl_socket;
    bool                                m_async;
    bool                                m_use_ssl;
};
