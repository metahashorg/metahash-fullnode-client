#include "http_json_rpc_request.h"
#include "settings/settings.h"
#include "log/log.h"
#include <iostream>

http_json_rpc_request::http_json_rpc_request(const std::string& host, asio::io_context& execute_context):
    m_io_ctx(execute_context),
    m_socket(m_io_ctx),
    m_resolver(m_io_ctx),
    m_duration(false, "json rpc"),
    m_host(host),
    m_ssl_ctx(ssl::context::sslv23),
    m_ssl_socket(m_io_ctx, m_ssl_ctx),
    m_async(true),
    m_use_ssl(false)
{
    std::string addr, path, port;
    utils::parse_address(m_host, addr, port, path, m_use_ssl);
    m_req.set(http::field::host, addr);
    m_req.set(http::field::user_agent, "metahash.service");
    m_req.set(http::field::content_type, "application/json");
    m_req.set(http::field::keep_alive, false);
    m_req.keep_alive(false);

    set_path(path);

    m_ssl_ctx.set_default_verify_paths();
    m_ssl_ctx.set_verify_mode(ssl::verify_fail_if_no_peer_cert);
    m_ssl_ctx.set_verify_callback(boost::bind(&http_json_rpc_request::verify_certificate, this, _1, _2));
}

http_json_rpc_request::~http_json_rpc_request()
{
    if (m_socket.is_open())
    {
        m_socket.shutdown(tcp::socket::shutdown_both);
        m_socket.close();
    }
    boost::system::error_code ec;
    m_ssl_socket.shutdown(ec);
}

void http_json_rpc_request::set_path(const std::string& path)
{
    std::string target = path;
    if (target[0] != '/')
        target.insert(target.begin(), '/');
    m_req.target(target);
}

void http_json_rpc_request::set_body(const std::string& body)
{
    beast::ostream(m_req.body())
        << body.c_str();
    m_req.set(http::field::content_length, m_req.body().size());
}

bool http_json_rpc_request::error_handler(const boost::system::error_code& e)
{
    if (!e)
        return false;
    m_timer.stop();
    if (m_socket.is_open())
    {
        m_socket.shutdown(tcp::socket::shutdown_both);
        m_socket.close();
    }
    boost::system::error_code ec;
    m_ssl_socket.shutdown(ec);
    if (e != asio::error::operation_aborted)
    {
        logg::push_err(e.message().c_str());
        m_result.set_error(32000, e.message().c_str());
        perform_callback();
    }

    STREAM_LOG_ERR("Request error:" << std::endl << m_result.stringify())

    if (!m_async && !m_io_ctx.stopped())
        m_io_ctx.stop();

    m_duration.stop();

    return true;
}

void http_json_rpc_request::execute()
{
    execute_async(nullptr);
    m_async = false;
    m_io_ctx.run();
}

void http_json_rpc_request::execute_async(http_json_rpc_execute_callback callback)
{
    m_duration.start();

    if (callback)
        m_callback = boost::bind(callback);

    m_timer.start(std::chrono::milliseconds(3000), boost::bind(&http_json_rpc_request::on_timer, this));

    std::string addr, path, port;
    utils::parse_address(m_host, addr, port, path, m_use_ssl);
    m_resolver.async_resolve(addr, port,
            boost::bind(&http_json_rpc_request::on_resolve, shared_from_this(), asio::placeholders::error, asio::placeholders::results));
}

void http_json_rpc_request::on_timer()
{
    STREAM_LOG_ERR("Request timeout")

    m_result.set_error(32001, "Request timeout");
    perform_callback();
    boost::system::error_code ec;
    m_socket.close(ec);
    m_ssl_socket.shutdown(ec);
    m_duration.stop();
}

void http_json_rpc_request::on_resolve(const boost::system::error_code& e, tcp::resolver::results_type eps)
{
    if (error_handler(e))
        return;

    asio::async_connect(is_ssl() ? m_ssl_socket.lowest_layer() : m_socket, eps,
        boost::bind(&http_json_rpc_request::on_connect, shared_from_this(), asio::placeholders::error, asio::placeholders::endpoint));
}

void http_json_rpc_request::on_connect(const boost::system::error_code& e, const tcp::endpoint& ep)
{
    if (error_handler(e))
        return;

    if (is_ssl())
    {
        m_ssl_socket.async_handshake(ssl::stream<tcp::socket>::client,
            boost::bind(&http_json_rpc_request::on_handshake, shared_from_this(), asio::placeholders::error));
    }
    else
    {
        STREAM_LOG_DBG("Send request " << m_host << " << " << beast::buffers_to_string(m_req.body().data()))

        http::async_write(m_socket, m_req,
            boost::bind(&http_json_rpc_request::on_write, shared_from_this(), asio::placeholders::error));
    }
}

void http_json_rpc_request::on_handshake(const boost::system::error_code& e)
{
    if (error_handler(e))
        return;

    STREAM_LOG_DBG("Send request: " << m_host << " << " << beast::buffers_to_string(m_req.body().data()))

    http::async_write(m_ssl_socket, m_req,
        boost::bind(&http_json_rpc_request::on_write, shared_from_this(), asio::placeholders::error));
}

void http_json_rpc_request::on_write(const boost::system::error_code& e)
{
    if (error_handler(e))
        return;
    if (is_ssl())
    {
        http::async_read(m_ssl_socket, m_buf, m_response,
            boost::bind(&http_json_rpc_request::on_read, shared_from_this(), asio::placeholders::error));
    }
    else
    {
        http::async_read(m_socket, m_buf, m_response,
            boost::bind(&http_json_rpc_request::on_read, shared_from_this(), asio::placeholders::error));
    }
}

void http_json_rpc_request::on_read(const boost::system::error_code& e)
{
    if (error_handler(e))
        return;

    m_timer.stop();

    http::status status = m_response.result();
    if (status != http::status::ok)
    {
        STREAM_LOG_DBG("Incorrect response http status: " << status)
    }

    const bool succ = m_result.parse(m_response.body());
    if (!succ)
    {
        STREAM_LOG_DBG("Response json parse error")
        if (status != http::status::ok)
        {
            std::ostringstream stream;
            stream << "Incorrect response http status: " << status;
            m_result.set_error(32002, stream.str().c_str());
        }
    }

    STREAM_LOG_DBG("Recieve response: " << m_host << " >> " << std::endl << m_result.stringify())

    perform_callback();

    if (!m_async && !m_io_ctx.stopped())
        m_io_ctx.stop();

    m_duration.stop();
}

void http_json_rpc_request::perform_callback()
{
    if (m_callback)
    {
        m_callback();
        m_callback = nullptr;
    }
}

std::string http_json_rpc_request::get_result()
{
    return m_result.stringify();
}

bool http_json_rpc_request::verify_certificate(bool preverified, ssl::verify_context& ctx)
{
  return true;
}
