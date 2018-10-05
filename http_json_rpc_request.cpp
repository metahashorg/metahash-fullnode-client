#include "http_json_rpc_request.h"
#include "settings/settings.h"
#include "log/log.h"

http_json_rpc_request::http_json_rpc_request(const std::string& host, asio::io_context& execute_context):
    m_io_ctx(execute_context),
    m_socket(m_io_ctx),
    m_resolver(m_io_ctx),
    m_host(host)
{
    std::string addr, port;
    utils::parse_address(m_host, addr, port);
    m_req.set(http::field::host, addr);
    m_req.set(http::field::user_agent, "metahash.service");
    m_req.set(http::field::content_type, "application/json");
    m_req.keep_alive(false);
}

http_json_rpc_request::~http_json_rpc_request()
{
    if (m_socket.is_open())
    {
        m_socket.shutdown(tcp::socket::shutdown_both);
        m_socket.close();
    }
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
    if (e != asio::error::operation_aborted)
    {
        logg::push_err(e.message().c_str());
        m_result.set_error(32000, e.message().c_str());
        perform_callback();
    }
    return true;
}

void http_json_rpc_request::execute()
{
    execute_async(nullptr);
    m_io_ctx.run();
}

void http_json_rpc_request::execute_async(http_json_rpc_execute_callback callback)
{
    if (callback)
        m_callback = boost::bind(callback);

    m_timer.start(std::chrono::milliseconds(3000), boost::bind(&http_json_rpc_request::on_timer, this));

    std::string host, port;
    utils::parse_address(m_host, host, port);
    m_resolver.async_resolve({host, port},
            boost::bind(&http_json_rpc_request::on_resolve, shared_from_this(),
            asio::placeholders::error,
            asio::placeholders::iterator));
}

void http_json_rpc_request::on_timer()
{
    logg::push_err("Request timeout");
    m_result.set_error(32001, "Request timeout");
    perform_callback();
    boost::system::error_code ec;
    m_socket.close(ec);
}

void http_json_rpc_request::on_resolve(const boost::system::error_code &e, tcp::resolver::iterator it)
{
    if (error_handler(e))
        return;
    tcp::endpoint ep = *it;
    asio::async_connect(m_socket, it,
        boost::bind(&http_json_rpc_request::on_connect, shared_from_this(), asio::placeholders::error, ++it));
}

void http_json_rpc_request::on_connect(const boost::system::error_code& e, tcp::resolver::iterator it)
{
    if (error_handler(e))
        return;
    http::async_write(m_socket, m_req,
        boost::bind(&http_json_rpc_request::on_write, shared_from_this(), asio::placeholders::error));
}

void http_json_rpc_request::on_write(const boost::system::error_code& e)
{
    if (error_handler(e))
        return;
    http::async_read(m_socket, m_buf, m_response,
        boost::bind(&http_json_rpc_request::on_read, shared_from_this(), asio::placeholders::error));
}

void http_json_rpc_request::on_read(const boost::system::error_code& e)
{
    if (error_handler(e))
        return;

    m_timer.stop();
    http::status status = m_response.result();
    if (status != http::status::ok)
    {
        std::ostringstream stream;
        stream << "Incorrect response http status: " << status;
        m_result.set_error(32002, stream.str().c_str());
    }
    else
    {
        m_result.parse(m_response.body());
    }
    perform_callback();
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
