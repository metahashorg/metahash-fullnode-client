#include "http_json_rpc_request.h"
#include "settings/settings.h"
#include <iostream>

#include "log.h"

#include <boost/asio/placeholders.hpp>
#include <boost/asio/connect.hpp>

#include "boost/random.hpp"

http_json_rpc_request::http_json_rpc_request(const std::string& host, asio::io_context& execute_context):
    m_io_ctx(execute_context),
    m_socket(m_io_ctx),
    m_resolver(m_io_ctx),
    m_duration(false, "json-rpc"),
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
    boost::system::error_code ec;
    if (m_socket.is_open())
    {
        m_socket.shutdown(tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
    m_ssl_socket.shutdown(ec);
}

void http_json_rpc_request::set_path(const std::string& path)
{
    m_id = path;
    m_duration.set_message("json-rpc[" + m_id + "]");
    std::string target = path;
    if (target[0] != '/')
        target.insert(target.begin(), '/');
    m_req.target(target);
}

void http_json_rpc_request::set_body(const std::string& body)
{
    m_req.body().assign(body);
    m_req.set(http::field::content_length, m_req.body().size());

    json_rpc_reader reader;
    if (reader.parse(body)) {
        m_result.set_id(reader.get_id());
    }
}

bool http_json_rpc_request::error_handler(const boost::system::error_code& e)
{
    if (!e)
        return false;

    m_timer.stop();
    m_connect_timer.stop();

    boost::system::error_code ec;
    if (m_socket.is_open())
    {
        m_socket.shutdown(tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }

    m_ssl_socket.shutdown(ec);

    LOGERR << "json-rpc[" << m_id << "] Request error: " << e.value() << " " << e.message();

    //if (e != asio::error::operation_aborted)
    {
        m_result.set_error(e.value(), "Request error: " + e.message());
        perform_callback();
    }

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

    if (callback) {
        m_callback = boost::bind(callback);
    }
    std::string addr, path, port;
    utils::parse_address(m_host, addr, port, path, m_use_ssl);
    m_resolver.async_resolve(addr, port,
            boost::bind(&http_json_rpc_request::on_resolve, shared_from_this(), asio::placeholders::error, asio::placeholders::results));
}

void http_json_rpc_request::on_request_timeout()
{
    LOGERR << "json-rpc[" << m_id << "] Request timeout" << settings::system::jrpc_timeout << " ms";

    m_connect_timer.stop();

    m_result.set_error(32001, "Request timeout " + std::to_string(settings::system::jrpc_timeout) + " ms");
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

    m_connect_timer.start(std::chrono::milliseconds(settings::system::jrpc_conn_timeout),
                          boost::bind(&http_json_rpc_request::on_connect_timeout, shared_from_this()));

    asio::async_connect(is_ssl() ? m_ssl_socket.lowest_layer() : m_socket, eps,
            boost::bind(&http_json_rpc_request::on_connect, shared_from_this(), asio::placeholders::error, asio::placeholders::endpoint));
}

void http_json_rpc_request::on_connect_timeout()
{
    LOGDEBUG << "json-rpc[" << m_id << "] Connection timeout " << settings::system::jrpc_conn_timeout << " ms to " << m_host;

    m_result.set_error(32002, "Connection timeout " + std::to_string(settings::system::jrpc_conn_timeout) + " ms " + m_host);
    perform_callback();
    boost::system::error_code ec;
    m_socket.close(ec);
    m_ssl_socket.shutdown(ec);
    m_duration.stop();

//    m_connect_timer.run_once();
    m_connect_timer.stop();
}

void http_json_rpc_request::on_connect(const boost::system::error_code& e, const tcp::endpoint& ep)
{
    m_connect_timer.stop();

    if (error_handler(e))
        return;

    m_timer.start(std::chrono::milliseconds(settings::system::jrpc_timeout),
                  boost::bind(&http_json_rpc_request::on_request_timeout, shared_from_this()));

    if (is_ssl())
    {
        m_ssl_socket.async_handshake(ssl::stream<tcp::socket>::client,
            boost::bind(&http_json_rpc_request::on_handshake, shared_from_this(), asio::placeholders::error));
    }
    else
    {
        LOGDEBUG << "json-rpc[" << m_id << "] Send request: " << m_host << " <<< " << m_req.body().c_str();

        http::async_write(m_socket, m_req,
            boost::bind(&http_json_rpc_request::on_write, shared_from_this(), asio::placeholders::error));
    }
}

void http_json_rpc_request::on_handshake(const boost::system::error_code& e)
{
    if (error_handler(e))
        return;

    LOGDEBUG << "json-rpc[" << m_id << "] Send request: " << m_host << " <<< " << m_req.body().c_str();

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
        LOGERR << "json-rpc[" << m_id << "] Incorrect response http status: " << status;
    }

    const bool succ = m_result.parse(m_response.body());
    if (!succ)
    {
        LOGERR << "json-rpc[" << m_id << "] Response json parse error: " << m_result.getDoc().GetParseError();
        if (status != http::status::ok)
        {
            std::ostringstream stream;
            stream << "Incorrect response http status: " << status;
            m_result.set_error(32002, stream.str().c_str());
        }
    }

    LOGDEBUG << "json-rpc[" << m_id << "] Recieve response: " << m_host << " >>> " << m_result.stringify();

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
