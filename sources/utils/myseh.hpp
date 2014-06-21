#pragma once

#include <windows.h>
#include <excpt.h>
#include <stdexcept>

/* Code using libseh should never be compiled using -fomit-frame-pointer */
#if defined(__GNUC__)
    #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
        #pragma GCC optimize ("no-omit-frame-pointer")
    #endif
#endif

namespace seh
{
    class Registrar;

    typedef int (*PEXCEPTION_HANDLER) (EXCEPTION_RECORD*, Registrar*, CONTEXT*, void*);

    class Registrar
    {
        private:
            Registrar*              prev;
            PEXCEPTION_HANDLER      handler;
            unsigned int            magic;
            const char*             debugfilename;
            const char*             logpath;
            int                     counter;
            bool                    withinHandler;
            EXCEPTION_RECORD        record;
            CONTEXT                 context;

        public:
            Registrar(const char* d = NULL, const char* p = NULL);
            ~Registrar();

            // Do not use:
            void copyData(const EXCEPTION_RECORD* pRec, const CONTEXT* pCon);
            std::string getExceptionText() const;
            void writeStackTraceFile();
    };

    class exception : public std::runtime_error
    {
        public:
            explicit exception(const std::string& what_arg);
    };

    void printPrettyStack(std::ostream& stream, const char* debugfilename, const CONTEXT* pContext);
}
