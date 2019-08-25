#include "http_session.h"
#include "task_handlers/task_handlers.h"
#include "task_handlers/base_handler.h"
#include "json_rpc.h"
#include "settings/settings.h"
#include "common/string_utils.h"
#include "common/log.h"
#include <boost/exception/all.hpp>


#define HTTP_SESS_BGN try

#define HTTP_SESS_END \
    catch (boost::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " boost exception: " << boost::diagnostic_information(ex);\
    } catch (std::exception& ex) {\
        LOGERR << __PRETTY_FUNCTION__ << " std exception: " << ex.what();\
    } catch (...) {\
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";\
    }

http_session::http_session(tcp::socket&& socket) :
    m_socket(std::move(socket)),
    m_http_ver(11),
    m_http_keep_alive(false)
{
}

http_session::~http_session()
{
    close();
}

void http_session::run()
{
    HTTP_SESS_BGN
    {
        m_req.clear();
        m_req.body().clear();
        m_buf.consume(m_buf.size());

        auto self = shared_from_this();
        http::async_read(m_socket, m_buf, m_req, [self](beast::error_code ec, std::size_t bytes_transferred)
        {
            if (!ec && bytes_transferred > 0)
            {
                self->process_request();
                if (self->keep_alive()) {
                    self->run();
                }
            }
        });
    }
    HTTP_SESS_END
}

asio::io_context& http_session::get_io_context()
{
    return m_socket.get_io_context();
}

void http_session::process_request()
{
    HTTP_SESS_BGN
    {
        m_http_keep_alive = false;

        if (!check_auth(m_req)) {
            boost::system::error_code ec;
            const tcp::endpoint& ep = m_socket.remote_endpoint(ec);
            if (ec) {
                LOGWARN << "Unknown client has not passed authentication";
            } else {
                LOGWARN << ep.address().to_string(ec) << ":" << ep.port() << " has not passed authentication";
            }
            send_bad_response(http::status::unauthorized, "Access Denied");
            return;
        }

        for (;;) {
            m_http_ver = m_req.version();
            auto field = m_req.find(http::field::connection);
            if (field != m_req.end() && field->value() == "close") {
                m_http_keep_alive = false;
                break;
            }
            if (m_http_ver == 11) {
                m_http_keep_alive = true;
                break;
            }
            m_http_keep_alive = m_req.keep_alive();
            break;
        }

        switch(m_req.method()) {
        case http::verb::post:
            process_post_request();
            break;
        case http::verb::get:
            process_get_request();
            break;
        default:
            send_bad_response(http::status::bad_request, "Incorrect HTTP Method");
            break;
        }
    }
    HTTP_SESS_END
}

void http_session::send_bad_response(http::status status, const char* error)
{
    HTTP_SESS_BGN
    {
        http::response<http::string_body> response;
        response.result(status);
        response.set(http::field::content_type, "text/plain");
        // TODO convertation into UTF-8
        if (error) {
            response.body().assign(error);
        }
        send_response(response);
    }
    HTTP_SESS_END
}

void http_session::send_json(const char* data, size_t size)
{
    HTTP_SESS_BGN
    {
        http::response<http::string_body> response;
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");
        // TODO convertation into UTF-8
        response.body().assign(data, size);
        send_response(response);
    }
    HTTP_SESS_END
}

void http_session::send_response(http::response<http::string_body>& response)
{
    HTTP_SESS_BGN
    {
        response.version(11);

        time_t dt = time(nullptr);
        struct tm tm = *gmtime(&dt);
        char buf[32] = {0};
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);

        response.set(http::field::date, buf);
        //response.set(http::field::server, "metahash.service");
        response.set(http::field::content_length, response.body().size());
        if (settings::service::keep_alive) {
            if (m_http_ver == 10) {
                response.set(http::field::connection, m_http_keep_alive ? "Keep-Alive" : "close");
            } else if (!m_http_keep_alive){
                response.set(http::field::connection, "close");
            }
        } else {
            response.set(http::field::connection, "close");
        }
        boost::system::error_code ec;
        http::write(m_socket, response, ec);
        if (ec) {
            LOGERR << __PRETTY_FUNCTION__ << " Error: " << ec.message();
        }
        if (!keep_alive()) {
            close();
        }
    }
    HTTP_SESS_END
}

void http_session::process_post_request()
{
    HTTP_SESS_BGN
    {
        if (m_req.target().size() != 1 || m_req.target()[0] != '/')
        {
            send_bad_response(http::status::bad_request, "Incorrect Path");
            return;
        }

        std::string_view json;
        json_rpc_reader reader;
        json_rpc_writer writer;

        // TODO
        // Add batch load

        if (reader.parse(m_req.body().c_str())) {
            const std::string_view method = reader.get_method();
            if (method.data() == nullptr) {
                LOGERR << "Method is not provided";
                writer.set_id(reader.get_id());
                writer.set_error(-32600, "JSON method is not provided");
                json = writer.stringify();
            } else {
                auto it = post_handlers.find(std::make_pair(method.data(), settings::system::useLocalDatabase));
                if (it == post_handlers.end()) {
                    LOGERR << "Incorrect service method: " << method;

                    writer.set_id(reader.get_id());
                    writer.set_error(-32601, string_utils::str_concat("Method '", method, "' does not exist").c_str());
                    json = writer.stringify();
                } else {
                    auto res = it->second(shared_from_this(), m_req.body());
                    // async operation
                    if (!res)
                        return;
                    json = res.message;
//                    json.append(res.message);
                }
            }
        } else {
            LOGERR << "Parse json error (" << reader.get_parse_error() << "): " << reader.get_parse_error_str() << ". Body: " << std::endl << m_req.body();
            writer.set_error(-32700, string_utils::str_concat(
                             "Parse json error (", std::to_string(reader.get_parse_error()),
                             "): ", reader.get_parse_error_str()).c_str());
            json = writer.stringify();
        }

        send_json(json.data(), json.size());
    }
    HTTP_SESS_END
}

void http_session::process_get_request()
{
    HTTP_SESS_BGN
    {
        if (m_req.target().size() < 2) {
            send_bad_response(http::status::bad_request, "Incorrect Path");
            return;
        }

        std::string_view params;
        std::string_view method(m_req.target().data(), m_req.target().size());
        size_t tmp = method.find_first_of('?');
        if (tmp != std::string_view::npos) {
            params = method.substr(tmp + 1, method.size() - tmp);
            method.remove_suffix(method.size() - tmp);
        }

        method.remove_prefix(1);

        std::string json;
        json_rpc_writer writer;
        auto it = get_handlers.find(method);
        if (it == get_handlers.end()) {
            LOGWARN << "Incorrect service method " << method;
            writer.set_id(1);
            writer.set_error(-32601, string_utils::str_concat("Method '", method, "' does not exist").c_str());
            json = writer.stringify();
        } else {
            writer.set_id(1);
            if (!params.empty()) {
                json_utils::to_json(params, *writer.get_params(), writer.get_allocator());
            }
            std::string_view t = writer.stringify();
            auto res = it->second(shared_from_this(), std::string(t.data(), t.size()));
            // async operation
            if (!res)
                return;
            json.append(res.message);
        }
        send_json(json.c_str(), json.size());
    }
    HTTP_SESS_END
}

void http_session::close()
{
    HTTP_SESS_BGN
    {
        boost::system::error_code ec;
        m_socket.shutdown(tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
    HTTP_SESS_END
}

bool http_session::keep_alive()
{
    if (settings::service::keep_alive) {
        return m_http_keep_alive;
    }
    return false;
}

bool http_session::check_auth(const http::request<http::string_body>& req)
{
    if (!settings::service::auth_enable) {
        return true;
    }
    auto field = req.find("x-auth-key");
    if (field != req.end()) {
        return field->value().compare(settings::service::auth_key) == 0;
    }
    return false;
}
