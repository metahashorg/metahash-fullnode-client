#pragma once

#include <string>
#include <sstream>
#include "log.h"

class invalid_param
{
public:
    invalid_param(std::string message): m_msg(message) {}
    ~invalid_param() {};

    const char* what() { return m_msg.c_str(); };

protected:
    std::string m_msg;
};

class parse_error: public invalid_param
{
public:
    parse_error(std::string message): invalid_param(message) {}

    const char* what() { return m_msg.c_str(); };
};

#define CHK_PARSE(condition, message) \
    if (!(condition)) {\
        throw parse_error(message); }

#define CHK_PRM(condition, message) \
    if (!(condition)) {\
        throw invalid_param(message); }

#define BGN_TRY try

#define END_TRY_RET_PARAM(ret, param) \
    catch (parse_error& ex) {\
        LOGERR << "ParseError Exception: \"" << ex.what() << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32700, ex.what());\
        param;\
        return ret;\
    } catch (invalid_param& ex) {\
        LOGERR << "InvalidParam Exception: \"" << ex.what() << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32602, ex.what());\
        param;\
        return ret;\
    } catch (const std::string& ex) {\
        LOGERR << "String Exception: \"" << ex << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32602, ex);\
        param;\
        return ret;\
    } catch (std::exception& ex) {\
        LOGERR << "STD Exception: \"" << ex.what() << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32602, ex.what());\
        param;\
        return ret;\
    } catch(...) {\
        LOGERR << "Unknown exception: (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32602, "Unknown exception");\
        param;\
        return ret;\
    }

#define END_TRY_RET(ret)\
    END_TRY_RET_PARAM(ret, )

#define END_TRY_PARAM(param)\
    END_TRY_RET_PARAM(,param)

#define END_TRY\
    END_TRY_RET()
