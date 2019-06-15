#pragma once

#include <string>
#include <sstream>
#include "log.h"

class invalid_param
{
public:
    invalid_param(std::string message): m_msg(message) {}
    invalid_param(std::string message, std::string additional): m_msg(message), m_addit(additional) {}
    ~invalid_param() {}

    const std::string& what() { return m_msg; }
    const std::string& additional() { return m_addit; }

protected:
    std::string m_msg;
    std::string m_addit;
};

class parse_error: public invalid_param
{
public:
    parse_error(std::string message): invalid_param(message) {}
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
        LOGERR << "ParseError Exception: \"" << ex.what().c_str() << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32700, ex.what());\
        param;\
        return ret;\
    } catch (invalid_param& ex) {\
        LOGERR << "InvalidParam Exception: \"" << ex.what().c_str() << "\" (" << __FILE__ << " : " << __LINE__ << ")";\
        this->m_writer.reset();\
        this->m_writer.set_error(-32602, ex.what());\
        if (!ex.additional().empty()) {\
            this->m_writer.add_error_data("Description", ex.additional()); }\
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
