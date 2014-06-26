#pragma once

#include <stdexcept>

class AssertException : public std::runtime_error
{
    public:
        explicit AssertException(const std::string& what_arg);
        static void throwInstance(const char* file, int line, const char* txt);
};

#ifdef assert
    #undef assert
#endif
#define assert(cond)    do { if (!(cond)) AssertException::throwInstance(__FILE__, __LINE__, #cond); } while (false)
