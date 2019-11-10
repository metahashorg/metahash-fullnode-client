#ifndef __BASE_HANDLER_H__
#define __BASE_HANDLER_H__

#include <memory>
#include "http_session_context_ptr.h"
#include "json_rpc.h"
#include "exception/except.h"
#include "time_duration.h"

using mh_count_t = uint64_t;
using handler_result = std::pair<bool, std::string>;

class base_handler: public std::enable_shared_from_this<base_handler> {
public:
    base_handler(session_context_ptr ctx)
        : m_context(ctx)
        , m_duration(false)
        , m_name(__func__)
    {
        m_result.first = false;
    }

    virtual ~base_handler() {
        m_duration.stop();
    }

public:
    bool prepare(const std::string& params);

    virtual void execute() = 0;

    handler_result result()
    {
        m_result.second = m_writer.stringify();
        return m_result;
    }

protected:
    template <typename T>
    std::shared_ptr<T> shared_from(T*) {
        return std::static_pointer_cast<T>(shared_from_this());
    }

    virtual bool prepare_params() = 0;

    const char* get_name() const {
        return m_name;
    }

protected:
    json_rpc_id             m_id = { 0 };
    handler_result          m_result;
    json_rpc_reader         m_reader;
    json_rpc_writer         m_writer;
    session_context_ptr     m_context;
    utils::time_duration    m_duration;
    const char*             m_name;
};

template <class T>
handler_result perform(session_context_ptr ctx, const std::string& params) {
    try {
        std::shared_ptr<T> obj = std::make_shared<T>(ctx);
        if (obj->prepare(params)) {
            obj->execute();
        }
        return obj->result();
    } catch (std::exception& ex) {
        LOGERR << __PRETTY_FUNCTION__ << " exception: " << ex.what();
        return handler_result();
    } catch (...) {
        LOGERR << __PRETTY_FUNCTION__ << " unhandled exception";
        return handler_result();
    }
}

#endif // __BASE_HANDLER_H__
