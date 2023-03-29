#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <string>

class SyntaxError 
    : public std::runtime_error 
{
public:
    using runtime_error::runtime_error;
};

class LispError
    :public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

class ExitEvent
    :public std::runtime_error
{
    int eCode;
public:
    using runtime_error::runtime_error;
    ExitEvent(int code)
        :eCode(code), runtime_error("") {}
    int exitCode() const;
};

class IntenalError
    :public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

#endif
