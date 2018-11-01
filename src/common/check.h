#ifndef CHECK_H_
#define CHECK_H_

#include <string>

using exception = std::string;

struct UserException {
    std::string exception;
    
    UserException(const std::string &exception)
        : exception(exception)
    {}
};

#define throwErr(s) { \
{ \
    throw exception((s) + std::string(". Error at file ") \
        + std::string(__FILE__) + std::string(" line ") + std::to_string(__LINE__)); \
} \
}

#define throwUserErr(s) { \
{ \
    throw UserException((s) + std::string(". Error at file ") \
        + std::string(__FILE__) + std::string(" line ") + std::to_string(__LINE__)); \
} \
}

#define CHECK(v, s) { \
if (!(v)) { \
    throwErr(s); \
} \
}

#define CHECK_USER(v, s) { \
if (!(v)) { \
    throwUserErr(s); \
} \
}

#endif // CHECK_H_
