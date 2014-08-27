#pragma once

#include <stdexcept>

class AssertBase
{
    public:
        virtual void writeDebugInformations(std::ostream& stream);
        void throwAssertException(const char* file, int line, const char* txt);
};

class AssertException : public std::runtime_error
{
    public:
        explicit AssertException(const std::string& what_arg);
};

void throwAssertException(const char* file, int line, const char* txt);

#ifdef assert
    #undef assert
#endif
#define assert(cond)    do { if (!(cond)) throwAssertException(__FILE__, __LINE__, #cond); } while (false)
