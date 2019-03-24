#include "http_json_rpc_request.h"
#include "settings/settings.h"
#include "common/string_utils.h"

#include "log.h"

#include <boost/asio/placeholders.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>

http_json_rpc_request::http_json_rpc_request(const std::string& host, asio::io_context& execute_context):
    m_io_ctx(execute_context),
    m_socket(m_io_ctx),
    m_resolver(m_io_ctx),
    m_duration(false, "json-rpc"),
    m_callback(nullptr),
    m_host(host),
    m_ssl_ctx(ssl::context::sslv23),
    m_ssl_socket(m_io_ctx, m_ssl_ctx),
    m_async(true),
    m_use_ssl(false),
    m_canceled(false)
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

    m_ssl_ctx.set_verify_callback([this](bool preverified, ssl::verify_context& ctx) -> bool {
        return this->verify_certificate(preverified, ctx);
    });
}

http_json_rpc_request::~http_json_rpc_request()
{
    close();
}

void http_json_rpc_request::set_path(const std::string& path)
{
    JRPC_BGN
    {
        m_id = path;
        m_duration.set_message("json-rpc[" + m_id + "]");
        std::string target = path;
        if (target[0] != '/')
            target.insert(target.begin(), '/');
        m_req.target(target);
    }
    JRPC_END()
}

void http_json_rpc_request::set_body(const std::string& body)
{
    JRPC_BGN
    {
        m_req.body().assign(body);
        m_req.set(http::field::content_length, m_req.body().size());

        json_rpc_reader reader;
        if (reader.parse(body)) {
            m_result.set_id(reader.get_id());
        }
    }
    JRPC_END()
}

bool http_json_rpc_request::error_handler(const boost::system::error_code& e, const char* from)
{
    JRPC_BGN
    {
        std::lock_guard<std::mutex> lock(m_locker);

        if (m_canceled) {
            return true;
        }

        if (!e) {
            return false;
        }

        m_canceled = true;

        m_timer.stop();
        m_connect_timer.stop();

        LOGERR << "json-rpc[" << m_id << "] Request error (" << from << "): " << e.value() << " " << e.message();

        close();

        //if (e != asio::error::operation_aborted)
        {
            m_result.set_error(-32603, string_utils::str_concat("Json-rpc ", from, " error ", std::to_string(e.value()), " : ", e.message()));
            perform_callback();
        }

        if (!m_async && !m_io_ctx.stopped())
            m_io_ctx.stop();

        m_duration.stop();

        return true;
    }
    JRPC_END(false)
}

void http_json_rpc_request::execute()
{
    JRPC_BGN
    {
        execute_async(nullptr);
        m_async = false;
        m_io_ctx.run();
    }
    JRPC_END()
}

void http_json_rpc_request::execute_async(http_json_rpc_execute_callback callback)
{
    JRPC_BGN
    {
        m_duration.start();

        if (callback) {
            m_callback = callback;
        }
        std::string addr, path, port;
        utils::parse_address(m_host, addr, port, path, m_use_ssl);

        auto self = shared_from_this();
        m_resolver.async_resolve(addr, port, [self](const boost::system::error_code& e, tcp::resolver::results_type eps) {
            self->on_resolve(e, eps);
        });
    }
    JRPC_END()
}

void http_json_rpc_request::on_request_timeout()
{
    JRPC_BGN
    {
        std::lock_guard<std::mutex> lock(m_locker);

        if (m_canceled) {
            return;
        }
        m_canceled = true;

        LOGERR << "json-rpc[" << m_id << "] Request timeout " << settings::system::jrpc_timeout << " ms";

        close();
        m_connect_timer.stop();
        m_timer.set_callback(nullptr);
        m_duration.stop();
        m_result.set_error(-32603,
            string_utils::str_concat("Json-rpc timeout ", std::to_string(settings::system::jrpc_timeout), " ms"));
        perform_callback();
    }
    JRPC_END()
}

void http_json_rpc_request::on_resolve(const boost::system::error_code& e, tcp::resolver::results_type eps)
{
    JRPC_BGN
    {
        if (error_handler(e, __func__)) {
            return;
        }

        auto self = shared_from_this();
        m_connect_timer.start(std::chrono::milliseconds(settings::system::jrpc_conn_timeout), [self](){
            self->on_connect_timeout();
        });

//        asio::socket_base::reuse_address reuseaddr(true);
//        if (is_ssl()) {
//            m_ssl_socket.lowest_layer().set_option(reuseaddr);
//        } else {
//            m_socket.set_option(reuseaddr);
//        }

        asio::async_connect(is_ssl() ? m_ssl_socket.lowest_layer() : m_socket, eps,
            [self](const boost::system::error_code& e, const tcp::endpoint& ep){ self->on_connect(e, ep); });
    }
    JRPC_END()
}

void http_json_rpc_request::on_connect_timeout()
{
    JRPC_BGN
    {
        std::lock_guard<std::mutex> lock(m_locker);

        if (m_canceled) {
            return;
        }
        m_canceled = true;

        LOGDEBUG << "json-rpc[" << m_id << "] Connection timeout " << settings::system::jrpc_conn_timeout << " ms to " << m_host;

        close();
        m_timer.stop();
        m_connect_timer.set_callback(nullptr);
        m_duration.stop();
        m_result.set_error(-32603,
            string_utils::str_concat("Json-rpc connection timeout ", std::to_string(settings::system::jrpc_conn_timeout), " ms ", m_host));
        perform_callback();
    }
    JRPC_END()
}

void http_json_rpc_request::on_connect(const boost::system::error_code& e, const tcp::endpoint& ep)
{
    JRPC_BGN
    {
        if (error_handler(e, __func__)) {
            return;
        }

        m_connect_timer.stop();

        auto self = shared_from_this();
        m_timer.start(std::chrono::milliseconds(settings::system::jrpc_timeout), [self](){
            self->on_request_timeout();
        });

        if (is_ssl()) {
            m_ssl_socket.async_handshake(ssl::stream<tcp::socket>::client, [self](const boost::system::error_code& e){
                self->on_handshake(e);
            });
        } else {
            LOGDEBUG << "json-rpc[" << m_id << "] Send request: " << m_host << " <<< " << m_req.body().c_str();

            http::async_write(m_socket, m_req, [self](const boost::system::error_code& e, std::size_t){
                self->on_write(e);
            });
        }
    }
    JRPC_END()
}

void http_json_rpc_request::on_handshake(const boost::system::error_code& e)
{
    JRPC_BGN
    {
        if (error_handler(e, __func__)) {
            return;
        }

        LOGDEBUG << "json-rpc[" << m_id << "] Send request: " << m_host << " <<< " << m_req.body().c_str();

        auto self = shared_from_this();
        http::async_write(m_ssl_socket, m_req, [self](const boost::system::error_code& e, std::size_t){
            self->on_write(e);
        });
    }
    JRPC_END()
}

void http_json_rpc_request::on_write(const boost::system::error_code& e)
{
    JRPC_BGN
    {
        if (error_handler(e, __func__)) {
            return;
        }

        m_response.body_limit((std::numeric_limits<std::uint64_t>::max)());

        auto self = shared_from_this();
        if (is_ssl()) {
            http::async_read(m_ssl_socket, m_buf, m_response, [self](const boost::system::error_code& e, size_t sz){
                self->on_read(e, sz);
            });
        } else {
            http::async_read(m_socket, m_buf, m_response, [self](const boost::system::error_code& e, size_t sz){
                self->on_read(e, sz);
            });
        }
    }
    JRPC_END()
}

void http_json_rpc_request::on_read(const boost::system::error_code& e, size_t sz)
{
    JRPC_BGN
    {
        if (error_handler(e, __func__)) {
            return;
        }

        m_timer.stop();
        close();

        http::status status = m_response.get().result();
        if (status != http::status::ok) {
            LOGWARN << "json-rpc[" << m_id << "] Incorrect response http status: " << status;
        }

        const bool succ = m_result.parse(m_response.get().body());
        if (!succ) {
            LOGERR << "json-rpc[" << m_id << "] Response json parse error: " << m_result.getDoc().GetParseError();
            if (status != http::status::ok) {
                m_result.set_error(-32603,
                    string_utils::str_concat("Incorrect response http status: ", std::to_string(static_cast<unsigned>(status))));
            }
        }

        LOGDEBUG << "json-rpc[" << m_id << "] Recieve response: " << m_host << " >>> " << m_result.stringify();

        m_duration.stop();

        perform_callback();

        if (!m_async && !m_io_ctx.stopped()) {
            m_io_ctx.stop();
        }
    }
    JRPC_END()
}

void http_json_rpc_request::perform_callback()
{
    JRPC_BGN
    {
        if (m_callback) {
            m_callback();
            m_callback = nullptr;
        }
    }
    JRPC_END()
}

std::string http_json_rpc_request::get_result()
{
    JRPC_BGN
    {
        return m_result.stringify();
    }
    JRPC_END("")
}

bool http_json_rpc_request::verify_certificate(bool, ssl::verify_context&)
{
    return true;
}

void http_json_rpc_request::close()
{
    JRPC_BGN
    {
        boost::system::error_code ec;
        if (m_socket.is_open()) {
            m_socket.shutdown(tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
        }
        m_ssl_socket.shutdown(ec);
    }
    JRPC_END()
}
