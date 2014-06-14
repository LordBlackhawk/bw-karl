#include "system-helper.hpp"
#include <cxxabi.h>
#include <cstdlib>

std::string demangle(const std::string& fname)
{
    int status = 0;
    char* realname = abi::__cxa_demangle(fname.c_str(), 0, 0, &status);
    if (realname == NULL)
        return fname;
    std::string result = realname;
    free(realname);
    return result;
}
