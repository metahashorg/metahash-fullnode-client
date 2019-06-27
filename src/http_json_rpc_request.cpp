#include "http_json_rpc_request.h"
#include "settings/settings.h"
#include "common/string_utils.h"
#include "connection_pool.h"
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

    m_req.version(11);
    m_req.set(http::field::host, addr);
    m_req.set(http::field::user_agent, "metahash.service");
    m_req.set(http::field::content_type, "application/json");

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

void http_json_rpc_request::set_host(const std::string& host)
{
    JRPC_BGN
    {
        m_host = host;
        std::string addr, path, port;
        utils::parse_address(m_host, addr, port, path, m_use_ssl);
        m_req.set(http::field::host, addr);
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

        LOGERR << "json-rpc[" << m_id << "] Request error (" << from << ") to " << m_host << " : " << e.value() << " " << e.message();

        //close(boost::asio::error::get_system_category().equivalent(e, e.value()));
        close(true);

        //if (e != asio::error::operation_aborted)
        {
            m_result.set_error(-32603, string_utils::str_concat("Json-rpc error ", std::to_string(e.value()), " : ", e.message()));
            m_result.add_error_data("when", from);
            m_result.add_error_data("host", m_host);
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

        m_canceled = false;
        m_result.reset();
        m_response.reset(new json_response_type());
        m_response->body_limit((std::numeric_limits<std::uint64_t>::max)());
        m_req.erase(http::field::connection);

        auto self = shared_from_this();
        bool need_connect = true;

        if (g_conn_pool->enable()) {
            m_pool_obj = g_conn_pool->checkout(m_host);
            if (!g_conn_pool->valid(m_pool_obj)) {
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
                    m_timer.start(std::chrono::milliseconds(settings::system::jrpc_timeout), [self](){
                        self->on_request_timeout();
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
                    LOGWARN << "json-rpc[" << m_id << "] Assign socket \"" << m_host << "\" error " << ec.value() << ": " << ec.message();
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

        LOGERR << "json-rpc[" << m_id << "] Request timeout " << settings::system::jrpc_timeout << " ms " << m_host;

        close(true);
        m_connect_timer.stop();
        m_timer.set_callback(nullptr);
        m_duration.stop();
        m_result.set_error(-32603,
            string_utils::str_concat("Json-rpc timeout ", std::to_string(settings::system::jrpc_timeout), " ms"));
        m_result.add_error_data("host", m_host);
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

        LOGERR << "json-rpc[" << m_id << "] Connection timeout " << settings::system::jrpc_conn_timeout << " ms to " << m_host;

        close(true);
        m_timer.stop();
        m_connect_timer.set_callback(nullptr);
        m_duration.stop();
        m_result.set_error(-32603,
            string_utils::str_concat("Json-rpc connection timeout ", std::to_string(settings::system::jrpc_conn_timeout), " ms"));
        m_result.add_error_data("host", m_host);
        perform_callback();
    }
    JRPC_END()
}

void http_json_rpc_request::on_connect(const boost::system::error_code& e, const tcp::endpoint&)
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

#ifdef _DEBUG_
        LOGDEBUG << "json-rpc[" << m_id << "] Send request: " << m_host << " <<< " << m_req.body().c_str();
#else
        LOGDEBUG << "json-rpc[" << m_id << "] Send request: " << m_host << " " << m_req.body().size() << " bytes";
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
        if (error_handler(e, __func__)) {
            return;
        }

        m_timer.stop();
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

        LOGDEBUG << "json-rpc[" << m_id << "] Recieve response: " << m_host << " payload " << m_response->get().body().size() << " bytes, keep-alive " << keep_alive;

        close(!keep_alive);

        http::status status = m_response->get().result();
        if (status != http::status::ok) {
            LOGWARN << "json-rpc[" << m_id << "] Incorrect response http status: " << status;
        }

        const bool succ = m_result.parse(m_response->get().body());
        if (!succ) {
            if (status != http::status::ok) {
                m_result.set_error(-32603,
                    string_utils::str_concat("Incorrect response http status: ", std::to_string(static_cast<unsigned>(status))));
                LOGERR << "json-rpc[" << m_id << "] Response json parse error: " << m_result.getDoc().GetParseError();
            }
        } else {
#ifdef _DEBUG_
            LOGDEBUG << "json-rpc[" << m_id << "] Recieve response: " << m_host << " >>> " << m_response->get().body();
#endif
        }

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

void http_json_rpc_request::close(bool force)
{
    JRPC_BGN
    {
        boost::system::error_code ec;
        if (g_conn_pool->enable() && g_conn_pool->valid(m_pool_obj)) {
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
                        LOGERR << "json-rpc[" << m_id << "] release socket error: " << ec.message();
                    }
                } else {
                    m_pool_obj->socket = -1;
                }
                g_conn_pool->checkin(m_pool_obj);
                m_socket.shutdown(tcp::socket::shutdown_both, ec);
                m_socket.close(ec);
            } else {
                if (m_socket.native_handle() != -1) {
                    m_pool_obj->socket = m_socket.release(ec);
                    if (ec) {
                        LOGERR << "json-rpc[" << m_id << "] release socket error: " << ec.message();
                    }
                } else {
                    m_pool_obj->socket = -1;
                }
                g_conn_pool->checkin(m_pool_obj);
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

json_response_type* http_json_rpc_request::get_response()
{
    return m_response.get();
}
