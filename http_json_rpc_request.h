#pragma once

#include <string.h>

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json_rpc.h"

namespace	asio    = boost::asio;
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
    void on_resolve(const boost::system::error_code& e, tcp::resolver::iterator it);
    void on_connect(const boost::system::error_code& e, tcp::resolver::iterator it);
    void on_write(const boost::system::error_code& e);
    void on_read(const boost::system::error_code& e);
    void on_timer(const boost::system::error_code& e);

    bool error_handler(const boost::system::error_code& e);

    void perform_callback();

private:
    asio::io_context&                   m_io_ctx;
    tcp::socket                         m_socket;
    tcp::resolver                       m_resolver;
    asio::deadline_timer                m_timer;
    http::request<http::dynamic_body>   m_req { http::verb::post, "/", 11 };
    http::response<http::string_body>   m_response;
    boost::beast::flat_buffer           m_buf { 8192 };
    json_rpc_writer                     m_result;
    http_json_rpc_execute_callback      m_callback;
    std::string                         m_host;
};
