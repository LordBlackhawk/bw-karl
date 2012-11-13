#include "exception-handler.hpp"
#include "stacktrace.hpp"
#include <iostream>

#define _WIN32_WINNT 0x0500
#include <windows.h>

namespace
{
    const char* exceptionCodeToString(DWORD code)
    {
        switch (code)
        {
            case EXCEPTION_ACCESS_VIOLATION:        return "Exception Access Violation";
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   return "Exception Array Bounds Exceeded";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:      return "Exception Float Divide by Zero";
            case EXCEPTION_INT_DIVIDE_BY_ZERO:      return "Exception Int Divide by Zero";
            case EXCEPTION_STACK_OVERFLOW:          return "Exception Stack Overflow";
            default:                                return "Unknown";
        }
    }

    void printExceptionRecord(EXCEPTION_RECORD* ExceptionRecord)
    {
        if (ExceptionRecord == NULL) {
            std::cout << "ExceptionRecord is NULL";
            return;
        }

        printStarLine();
        std::cout << "catch exception of type " << exceptionCodeToString(ExceptionRecord->ExceptionCode)
                  << " (id = " << ExceptionRecord->ExceptionCode << ").\n";

        DebugInfo debuginfo = readDebugInfo(ExceptionRecord->ExceptionAddress);
        std::cout << "in file '" << debuginfo.file << "' in method '" << debuginfo.method << "':" << debuginfo.line
                  << " (" << ExceptionRecord->ExceptionAddress << ").\n";
        if (ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
            const char* reason = (ExceptionRecord->ExceptionInformation[0] == 0) ? "read from" :
                                 (ExceptionRecord->ExceptionInformation[0] == 1) ? "write to" : "execute ";
            std::cout << "process tried to " << reason << " " << (void*)ExceptionRecord->ExceptionInformation[1] << ".\n";
        }
        
    }

    void printExceptionInfo(EXCEPTION_POINTERS* ExceptionInfo)
    {
        if (ExceptionInfo == NULL) {
            std::cout << "ExceptionInfo is NULL!";
            return;
        }
        printExceptionRecord(ExceptionInfo->ExceptionRecord);
    }

    LONG WINAPI OurSilentCrashHandler(EXCEPTION_POINTERS* /*ExceptionInfo*/)
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    int inCrashHandlerLevel = 0;

    LONG CALLBACK OurCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
    {
        if (inCrashHandlerLevel > 3)
            return EXCEPTION_EXECUTE_HANDLER;
        ++inCrashHandlerLevel;
        printExceptionInfo(ExceptionInfo);
        printStarLine(); //plotStackTrace(1);
        --inCrashHandlerLevel;
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

void ExceptionHandlerCode::onProgramStart(const char* programname)
{
    SetUnhandledExceptionFilter(OurSilentCrashHandler);
    AddVectoredExceptionHandler(1, OurCrashHandler);
    setDebugInfoFile(programname);
}
