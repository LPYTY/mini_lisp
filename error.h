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
public:
    using runtime_error::runtime_error;
    ExitEvent(int code)
        :runtime_error(std::to_string(code)) {}
    int exitCode() const;
};

#endif
