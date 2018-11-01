#pragma once

#include <memory>
#include "../http_session.h"
#include "../json_rpc.h"
#include "../cpplib_open_ssl_decor/crypto.h"
#include "../settings/settings.h"
#include "../log/log.h"
#include "../exception/except.h"
#include "utils.h"

using mh_count_t = uint64_t;

struct handler_result
{
    operator bool() const                 {return !pending;}
    operator const std::string() const    {return message;}
    bool pending = { false };
    std::string message;
};

template <class T>
class base_handler
{
    friend struct handler_result;

public:
    base_handler(http_session_ptr session):
        m_session(session),
        m_duration(false)
    {
    }
    virtual ~base_handler()
    {
        m_duration.stop();
    }

    static handler_result perform(http_session_ptr session, const std::string& params)
    {
        try
        {
            std::shared_ptr<T> obj = std::make_shared<T>(session);
            if (obj->prepare(params))
                obj->execute();
            return obj->result();
        }
        catch (std::exception& ex)
        {
            STREAM_LOG_ERR(__PRETTY_FUNCTION__ << " exception: " << ex.what())
            return handler_result();
        }
        catch (...)
        {
            STREAM_LOG_ERR(__PRETTY_FUNCTION__ << " unhandled exception")
            return handler_result();
        }
    }

protected:
    bool prepare(const std::string& params);

    virtual bool prepare_params() = 0;
    virtual void execute() = 0;

    handler_result result()
    {
        this->m_result.message = this->m_writer.stringify();
        return this->m_result;
    }

protected:
    json_rpc_id             m_id = { 0 };
    handler_result          m_result;
    json_rpc_reader         m_reader;
    json_rpc_writer         m_writer;
    http_session_ptr        m_session;
    utils::time_duration    m_duration;
};

#define DECL_BASE_HANDLER(cl)\
    class cl : public base_handler<cl>\
    {\
    public:\
        typedef base_handler<cl> base;\
        cl(http_session_ptr session): base(session)\
        {\
            std::stringstream ss;\
            ss << __FUNCTION__;\
            m_duration.set_message(ss.str());\
        }\
        virtual ~cl() override {}\
        virtual bool prepare_params() override;\
        virtual void execute() override;\
    };