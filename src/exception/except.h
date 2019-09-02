//#pragma once

#include <string>
#include <sstream>
#include "log.h"

class invalid_param
{
public:
    invalid_param(const char* message):
        m_msg(message),
        m_code(-32602) {}
    invalid_param(const char* message, const char* additional):
        m_msg(message),
        m_addit(additional),
        m_code(-32602){}
    ~invalid_param() {}

    const char* what() { return m_msg.c_str(); }
    const char* additional() { return m_addit.empty() ? nullptr : m_addit.c_str(); }
    int get_code() { return m_code; }

protected:
    std::string m_msg;
    std::string m_addit;
    int         m_code;
};

class parse_error: public invalid_param
{
public:
    parse_error(const char* message): invalid_param(message) {
        m_code = -32700;
    }
};

class invalid_request: public invalid_param
{
public:
    invalid_request(const char* message): invalid_param(message) {
        m_code = -32600;
    }
};

#define CHK_PARSE(condition, message) \
    if (!(condition)) {\
        throw parse_error(message); }

#define CHK_PRM(condition, message) \
    if (!(condition)) {\
        throw invalid_param(message); }

#define CHK_REQ(condition, message) \
    if (!(condition)) {\
        throw invalid_request(message); }

#define BGN_TRY try

#define END_TRY(param) \
    catch (invalid_param& ex) {\
        LOGERR << "[" << get_name() << "] Error: " << ex.what() << " (" << __FILE__ << " : " << __LINE__ << ")";\
        m_writer.reset();\
        m_writer.set_error(ex.get_code(), ex.what());\
        if (ex.additional()) {\
            m_writer.add_error_data("description", ex.additional()); }\
        param;\
    } catch (const std::string& ex) {\
        LOGERR << "[" << get_name() << "] String Exception: \"" << ex << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        m_writer.reset();\
        m_writer.set_error(-32602, ex.c_str());\
        param;\
    } catch (std::exception& ex) {\
        LOGERR << "[" << get_name() << "] STD Exception: \"" << ex.what() << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        m_writer.reset();\
        m_writer.set_error(-32603, ex.what());\
        param;\
    } catch(...) {\
        LOGERR << "[" << get_name() << "] Unknown exception: (" << __FILE__ << " : " << __LINE__ << ")";\
        m_writer.reset();\
        m_writer.set_error(-32603, "Unknown exception");\
        param;\
    }
