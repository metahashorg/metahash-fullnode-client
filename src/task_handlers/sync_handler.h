#ifndef SYNC_HANDLER_H_
#define SYNC_HANDLER_H_

#include <string>
#include "base_handler.h"
#include "../http_json_rpc_request.h"

template <class T>
class base_sync_handler : public base_handler<T>, public std::enable_shared_from_this<T>
{
public:
    base_sync_handler(const std::string& host, http_session_ptr session): base_handler<T>(session)
    {
        m_request = std::make_shared<http_json_rpc_request>(host, session->get_io_context());
    }
    virtual ~base_sync_handler() override { }
    
    virtual void execute() override;
    
protected:
    
    virtual void executeImpl() = 0;
       
protected:
    bool                        m_async_execute = {true};
    http_json_rpc_request_ptr   m_request;
};

#define DECL_SYNC_HANDLER(cl)\
class cl : public base_sync_handler<cl>\
{\
    public:\
        typedef base_sync_handler<cl> base;\
        cl(http_session_ptr session): base(settings::server::tor, session)\
        {\
            std::stringstream ss;\
            ss << __FUNCTION__;\
            m_duration.set_message(ss.str());\
        }\
        virtual ~cl() override {}\
        virtual bool prepare_params() override;\
};


#endif // SYNC_HANDLER_H_
