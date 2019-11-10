#include "http_json_rpc_request.h"
#include "settings/settings.h"
#include "common/string_utils.h"
#include "connection_pool.h"

#include <boost/asio/placeholders.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>

http_json_rpc_request::http_json_rpc_request(std::string&& host, asio::io_context* ctx /*= nullptr*/, int timeout /*= -1*/, int conn_timeout /*= -1*/, int attempts_count /*= -1*/):
    m_io_ctx(ctx == nullptr ? new boost::asio::io_context(1) : nullptr),
    m_socket(ctx == nullptr ? *m_io_ctx : *ctx),
    m_resolver(ctx == nullptr ? *m_io_ctx : *ctx),
    m_duration(false, "json-rpc"),
    m_req(http::verb::post, "/", 11),
    m_callback(nullptr),
    m_host(std::move(host)),
    m_ssl_ctx(ssl::context::sslv23),
    m_ssl_socket(ctx == nullptr ? *m_io_ctx : *ctx, m_ssl_ctx),
    m_attempt(0),
    m_timeout(timeout > 0 ? static_cast<unsigned>(timeout) : settings::system::jrpc_timeout),
    m_conn_timeout(conn_timeout > 0 ? static_cast<unsigned>(conn_timeout) : settings::system::jrpc_conn_timeout),
    m_attempts_count(attempts_count > 0 ? static_cast<unsigned>(attempts_count) : settings::system::jrpc_attempts_count),
    m_use_ssl(false),
    m_canceled(false),
    m_rerun(false)
{
    m_req.version(11);
    m_req.set(http::field::content_type, "application/json");

    if (!m_host.empty()) {
        std::string addr, path, port;
        utils::parse_address(m_host, addr, port, path, m_use_ssl);
        m_req.set(http::field::host, addr);
//        set_path(path.c_str());
    }
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

void http_json_rpc_request::set_path(const char* path)
{
    JRPC_BGN
    {
        if (path == nullptr) {
            path = "/";
        }
        m_duration.set_message(string_utils::str_concat("json-rpc[", path, "]"));
        if (*path != '/') {
            m_req.target(string_utils::str_concat("/", path));
        } else {
            m_req.target(path);
        }
    }
    JRPC_END()
}

void http_json_rpc_request::reset_attempts()
{
    m_attempt = 0;
}

bool http_json_rpc_request::error_handler(const boost::system::error_code& e, const char* from)
{
    JRPC_BGN
    {
        std::lock_guard<std::mutex> lock(m_locker);

        if (!m_rerun) {
            if (m_canceled) {
                return true;
            }

            if (!e) {
                return false;
            }
        } else {
            m_rerun = false;
        }

        m_canceled = true;

        m_locker.unlock();

        m_timer.stop();

        LOGERR << "json-rpc[" << m_req.target() << "] Request error (" << from << ") to " << m_host << " : " << e.value() << " " << e.message();

        close(true);

        m_duration.stop();

        if (m_attempt < m_attempts_count) {
            LOGINFO << "json-rpc[" << m_req.target() << "] Rerun request";
            execute(m_callback);
        } else {
            m_result.set_error(-32603, string_utils::str_concat("Json-rpc error ", std::to_string(e.value()), " : ", e.message()).c_str());
            m_result.add_error_data("where", from);
            m_result.add_error_data("host", m_host);
            perform_callback();

            if (m_io_ctx.get() && !m_io_ctx->stopped())
                m_io_ctx->stop();
        }

        return true;
    }
    JRPC_END(false)
}

void http_json_rpc_request::execute(http_json_rpc_execute_callback callback)
{
    JRPC_BGN
    {
        m_attempt++;

        if (m_io_ctx.get() && m_attempt == 1) {
            m_io_ctx->restart();
        }

        m_duration.start();

        if (callback) {
            m_callback = callback;
        }

        LOGDEBUG << "json-rpc[" << m_req.target() << "] Run request attempt " << m_attempt << "/" << m_attempts_count;

        m_canceled = false;
        m_result.reset();
        m_response.reset(new json_response_type());
        m_response->body_limit((std::numeric_limits<std::uint64_t>::max)());
        m_req.erase(http::field::connection);

        auto self = shared_from_this();
        bool need_connect = true;

        if (socket_pool::get()->enable()) {
            m_pool_obj = socket_pool::get()->checkout(m_host);
            if (!socket_pool::get()->valid(m_pool_obj)) {
                m_req.set(http::field::connection, "close");
            } else if (m_pool_obj->socket != -1) {
                boost::system::error_code ec;
                tcp::endpoint ep;
                for (;;) {
                    if (is_ssl()) {
                        m_ssl_socket.lowest_layer().assign(tcp::v4(), m_pool_obj->socket, ec);
                        if (ec) break;
                        ep = m_ssl_socket.lowest_layer().remote_endpoint(ec);
                        if (ec) break;
                    } else {
                        m_socket.assign(tcp::v4(), m_pool_obj->socket, ec);
                        if (ec) break;
                        ep = m_socket.remote_endpoint(ec);
                        if (ec) break;
                    }
                    break;
                }

                if (!ec) {
                    need_connect = false;
                    m_timer.start(std::chrono::milliseconds(m_timeout), [self](){
                        self->on_timeout(timeout::request);
                    });
                    if (is_ssl()) {
                        http::async_write(m_ssl_socket, m_req, [self](const boost::system::error_code& e, std::size_t){
                            self->on_write(e);
                        });
                    } else {
                        http::async_write(m_socket, m_req, [self](const boost::system::error_code& e, std::size_t){
                            self->on_write(e);
                        });
                    }
                } else {
                    LOGWARN << "json-rpc[" << m_req.target() << "] Assign socket \"" << m_host << "\" error " << ec.value() << ": " << ec.message();
                    m_socket.shutdown(tcp::socket::shutdown_both, ec);
                    m_socket.close(ec);
                    m_ssl_socket.lowest_layer().close(ec);
                }
            }
        } else {
            m_req.set(http::field::connection, "close");
        }

        if (need_connect) {
            std::string addr, path, port;
            utils::parse_address(m_host, addr, port, path, m_use_ssl);

            m_resolver.async_resolve(addr, port, [self](const boost::system::error_code& e, tcp::resolver::results_type eps) {
                self->on_resolve(e, eps);
            });
        }

        if (m_io_ctx.get() && m_attempt == 1) {
            m_io_ctx->run();
        }
    }
    JRPC_END()
}

void http_json_rpc_request::on_timeout(timeout type)
{
    JRPC_BGN
    {
        std::lock_guard<std::mutex> lock(m_locker);

        if (m_canceled) {
            return;
        }
        m_canceled = true;

        close(true);

        m_timer.set_callback(nullptr);

        switch (type) {
        case timeout::request:
            LOGERR << "json-rpc[" << m_req.target() << "] Request timeout " << m_timeout << " ms " << m_host;
            break;
        case timeout::connection:
            LOGERR << "json-rpc[" << m_req.target() << "] Connection timeout " << m_conn_timeout << " ms to " << m_host;
            break;
        }

        m_duration.stop();
        if (m_attempt < m_attempts_count) {
            m_rerun = true;
        } else {
            switch (type) {
            case timeout::request:
                m_result.set_error(-32603,
                    string_utils::str_concat("Json-rpc timeout ", std::to_string(m_timeout), " ms").c_str());
                break;
            case timeout::connection:
                m_result.set_error(-32603,
                    string_utils::str_concat("Json-rpc connection timeout ", std::to_string(m_conn_timeout), " ms").c_str());
                break;
            }
            m_result.add_error_data("host", m_host);

            perform_callback();

            if (m_io_ctx.get() && !m_io_ctx->stopped())
                m_io_ctx->stop();
        }
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
        m_timer.start(std::chrono::milliseconds(m_conn_timeout), [self](){
            self->on_timeout(timeout::connection);
        });

        asio::async_connect(is_ssl() ? m_ssl_socket.lowest_layer() : m_socket, eps,
            [self](const boost::system::error_code& e, const tcp::endpoint& ep){ self->on_connect(e, ep); });
    }
    JRPC_END()
}

void http_json_rpc_request::on_connect(const boost::system::error_code& e, const tcp::endpoint&)
{
    JRPC_BGN
    {
        m_timer.stop();

        if (error_handler(e, __func__)) {
            return;
        }

        auto self = shared_from_this();
        m_timer.start(std::chrono::milliseconds(m_timeout), [self](){
            self->on_timeout(timeout::request);
        });

        if (is_ssl()) {
            m_ssl_socket.async_handshake(ssl::stream<tcp::socket>::client, [self](const boost::system::error_code& e){
                self->on_handshake(e);
            });
        } else {
#ifdef _DEBUG_
            LOGDEBUG << "json-rpc[" << m_req.target() << "] Send request: " << m_host << " <<< " << m_req.body().c_str();
#else
            LOGDEBUG << "json-rpc[" << m_req.target() << "] Send request: " << m_host << " " << m_req.body().size() << " bytes";
#endif
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

#ifdef _DEBUG_
        LOGDEBUG << "json-rpc[" << m_req.target() << "] Send ssl request: " << m_host << " <<< " << m_req.body().c_str();
#else
        LOGDEBUG << "json-rpc[" << m_req.target() << "] Send ssl request: " << m_host << " " << m_req.body().size() << " bytes";
#endif

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

        auto self = shared_from_this();
        if (is_ssl()) {
            http::async_read(m_ssl_socket, m_buf, *m_response, [self](const boost::system::error_code& e, size_t sz){
                self->on_read(e, sz);
            });
        } else {
            http::async_read(m_socket, m_buf, *m_response, [self](const boost::system::error_code& e, size_t sz){
                self->on_read(e, sz);
            });
        }
    }
    JRPC_END()
}

void http_json_rpc_request::on_read(const boost::system::error_code& e, size_t)
{
    JRPC_BGN
    {
        m_timer.stop();

        if (error_handler(e, __func__)) {
            return;
        }

        m_duration.stop();

        bool keep_alive = true;
        for (;;) {
            auto field = m_response->get().find(http::field::connection);
            if (field != m_response->get().end() && field->value() == "close") {
                keep_alive = false;
                break;
            }
            if (m_response->get().version() == 11) {
                keep_alive = true;
                break;
            }
            keep_alive = m_response->get().keep_alive();
            break;
        }

        LOGDEBUG << "json-rpc[" << m_req.target() << "] Recieve response: " << m_host << " payload " << m_response->get().body().size() << " bytes, keep-alive " << keep_alive;

        close(!keep_alive);

        http::status status = m_response->get().result();
        if (status != http::status::ok) {
            LOGWARN << "json-rpc[" << m_req.target() << "] Incorrect response http status: " << status;
        }

        const bool succ = m_result.parse(m_response->get().body().c_str(), m_response->get().body().size());
        if (!succ) {
            if (status != http::status::ok) {
                m_result.set_error(-32603,
                    string_utils::str_concat("Incorrect response http status: ", std::to_string(static_cast<unsigned>(status))).c_str());
                LOGERR << "json-rpc[" << m_req.target() << "] Response json parse error: " << m_result.get_doc().GetParseError();
            }
        } else {
#ifdef _DEBUG_
            LOGDEBUG << "json-rpc[" << m_req.target() << "] Recieve response: " << m_host << " >>> " << m_response->get().body();
#endif
        }

        perform_callback();

        if (m_io_ctx.get() && !m_io_ctx->stopped()) {
            m_io_ctx->stop();
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

const std::string_view http_json_rpc_request::get_result()
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

void http_json_rpc_request::close(bool force)
{
    JRPC_BGN
    {
        boost::system::error_code ec;
        if (socket_pool::get()->enable() && socket_pool::get()->valid(m_pool_obj)) {
            if (force) {
                m_socket.shutdown(tcp::socket::shutdown_both, ec);
                m_socket.close(ec);
                m_ssl_socket.shutdown(ec);
                ec.clear();
            }
            if (is_ssl()) {
                if (m_ssl_socket.lowest_layer().native_handle() != -1) {
                    m_pool_obj->socket = m_ssl_socket.lowest_layer().release(ec);
                    if (ec) {
                        LOGERR << "json-rpc[" << m_req.target() << "] release socket error: " << ec.message();
                    }
                } else {
                    m_pool_obj->socket = -1;
                }
                socket_pool::get()->checkin(m_pool_obj);
                m_socket.shutdown(tcp::socket::shutdown_both, ec);
                m_socket.close(ec);
            } else {
                if (m_socket.native_handle() != -1) {
                    m_pool_obj->socket = m_socket.release(ec);
                    if (ec) {
                        LOGERR << "json-rpc[" << m_req.target() << "] release socket error: " << ec.message();
                    }
                } else {
                    m_pool_obj->socket = -1;
                }
                socket_pool::get()->checkin(m_pool_obj);
                m_ssl_socket.shutdown(ec);
            }
        } else {
            m_socket.shutdown(tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
            m_ssl_socket.shutdown(ec);
        }
    }
    JRPC_END()
}

const json_response_type* http_json_rpc_request::get_response()
{
    return m_response.get();
}
