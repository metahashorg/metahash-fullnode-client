#pragma once

#include <string>
#include <sstream>
#include "../log/log.h"

class invalid_param
{
public:
    invalid_param(std::string message): m_msg(message)
    {
        std::stringstream ss;
        ss << __PRETTY_FUNCTION__ << " in file " << __FILE__ << " at line " << __LINE__;
        m_where = ss.str();
    }

    ~invalid_param() {};

    std::string what() { return m_msg; };
    std::string where() { return m_where; };

protected:
    std::string m_msg;
    std::string m_where;
};

#define CHK_PRM(condition, message) \
    if (!(condition)) {\
        std::ostringstream stream;\
        stream << message;\
        stream.flush();\
        throw invalid_param(stream.str()); }

#define BGN_TRY try

#define END_TRY_RET_PARAM(ret, param) \
    catch (invalid_param& ex)\
    {\
        STREAM_LOG_ERR("Exception \"" << ex.what() << "\" in func " << __PRETTY_FUNCTION__)\
        this->m_writer.reset();\
        this->m_writer.set_error(-32666, ex.what());\
        param;\
        return ret;\
    }\
    catch (std::exception& ex)\
    {\
        STREAM_LOG_ERR("Exception \"" << ex.what() << "\" in func " << __PRETTY_FUNCTION__)\
        this->m_writer.reset();\
        this->m_writer.set_error(-32666, ex.what());\
        param;\
        return ret;\
    }\
    catch(...)\
    {\
        STREAM_LOG_ERR("Unhandled exception in func " << __PRETTY_FUNCTION__)\
        this->m_writer.reset();\
        this->m_writer.set_error(-32666, "Unhandled exception");\
        param;\
        return ret;\
    }

#define END_TRY_RET(ret)\
    END_TRY_RET_PARAM(ret, )

#define END_TRY_PARAM(param)\
    END_TRY_RET_PARAM(,param)

#define END_TRY\
    END_TRY_RET()
