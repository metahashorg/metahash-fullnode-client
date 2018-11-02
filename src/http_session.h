#pragma once

#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

namespace	asio    = boost::asio;
namespace	ip      = boost::asio::ip;
using		tcp     = boost::asio::ip::tcp;
namespace   beast   = boost::beast;
namespace	http    = boost::beast::http;

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
        void send_response(http::response<http::dynamic_body>& response);
        void send_bad_request(const char* error);

private:
        tcp::socket                      m_socket;
        beast::flat_buffer               m_buf{ 8192 };
        http::request<http::string_body> m_req;
};
