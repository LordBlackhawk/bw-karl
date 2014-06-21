#include "addr2line.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cxxabi.h>
#include <sstream>

namespace
{
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

    struct DebugInfo
    {
        std::string method;
        std::string file;
        int         line;
    };

    bool readSingle(DebugInfo& di, FILE* f)
    {
        char buf1[1024], buf2[1024];
        int read = fscanf(f, "%1023s\n%1023s\n", &buf1[0], &buf2[0]);
        
        if ((read < 2) || (strlen(buf2) < 3))
            return false;

        buf2[1023] = 0;
        int p = 2;
        while ((buf2[p] != ':') && (buf2[p] != 0))
            ++p;

        if (buf2[p] != ':')
            return false;

        di.line   = atoi(&buf2[p+1]);
        buf2[p]    = 0;

        di.method = buf1;
        di.file   = buf2;
        
        if ((di.method == "??") && (di.file == "??"))
            return false;
        
        if ((di.method != "??") && (di.method != "main"))
            di.method = demangle("_" + di.method);

        return true;
    }
}

std::string readDebugInfo(const char* debugfilename, void* addr)
{
    char filename[1024];
    sprintf(filename, "addr2line -C -e %s -f -s -i %lx", debugfilename, (unsigned long) addr);
    FILE* f = popen (filename, "r");

    if (f == NULL)
        return "";

    std::stringstream stream;
    DebugInfo res;
    bool first = true;
    while (readSingle(res, f)) {
        if (!first)
            stream << ", ";
        stream
            << res.file
            << ":"
            << res.line
            << " "
            << res.method;
        first = false;
    }
    
    pclose(f);
    return stream.str();
}