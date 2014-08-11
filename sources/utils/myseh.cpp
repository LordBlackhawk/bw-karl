#include "myseh-internal.hpp"
#include <sstream>
#include <fstream>

namespace
{
    void __libseh_caller()
    {
        throw seh::exception(__libseh_get_registration()->getExceptionText());
    }

    int __libseh_exception_handler(EXCEPTION_RECORD* pRecord, seh::Registrar* pReg, CONTEXT* pContext, void*)
    {
        // 1. Copy informations
        pReg->copyData(pRecord, pContext);

        // 2. Write stack trace file
        pReg->writeStackTraceFile();

        // 3. Remove other exception handler
        while (__libseh_get_registration() != pReg)
            __libseh_pop_registration();

        // 4. Prepare stack for function '__libseh_caller'
        unsigned int* Esp = (unsigned int*)pContext->Esp;
        #define PUSH(value) do { --Esp; *Esp = (unsigned int)(value); } while (false)
        PUSH( pContext->Eip );
        PUSH( &__libseh_caller );
        PUSH( pContext->Ebp );
        #undef PUSH

        // 5. Return to prepared stack
        __libseh_unwind_to(Esp);

        // 6. Never reached
        return ExceptionContinueSearch;
    }
}

namespace seh
{
    Registrar::Registrar(const char* d, const char* p)
    {
        prev = __libseh_get_registration();
        handler = __libseh_exception_handler;
        magic = SEH_MAGIC_NUMBER;
        debugfilename = d;
        logpath = p;
        counter = 1;
        withinHandler = false;
        __libseh_set_registration(this);
    }

    Registrar::~Registrar()
    {
        __libseh_pop_registration();
    }

    void Registrar::copyData(const EXCEPTION_RECORD* pRec, const CONTEXT* pCon)
    {
        // Only continue if stack seems to be not corrupted
        if (magic != SEH_MAGIC_NUMBER)
            return;
        memcpy(&record, pRec, sizeof(record));
        memcpy(&context, pCon, sizeof(context));
    }

    std::string Registrar::getExceptionText() const
    {
        // Only continue if stack seems to be not corrupted
        if (magic != SEH_MAGIC_NUMBER)
            return "Stack is corrupted";
        // Special treatment for stack overflow (we have not enough stack to do something else)
        if (record.ExceptionCode == EXCEPTION_STACK_OVERFLOW)
            return "Stack overflow";

        std::string location = (debugfilename != NULL) ? readDebugInfo(debugfilename, (void*)context.Eip) : "unknown";
        std::stringstream stream;
        stream
            << std::hex << std::setfill('0')
            << "At 0x" << std::setw(8) << context.Eip << " [" << location << "]: ";
        switch(record.ExceptionCode)
        {
            case EXCEPTION_ACCESS_VIOLATION:
                stream
                    << "Access violation while "
                    << ((record.ExceptionInformation[0] == 0) ? "reading from " : (record.ExceptionInformation[0] == 1) ? "writing to " : "executing ")
                    << "0x" << std::setw(8) << record.ExceptionInformation[1]
                    << ".";
                break;

            #define HANDLE(id, txt)     case id: stream << txt; break;
            HANDLE(EXCEPTION_DATATYPE_MISALIGNMENT, "Data type misalignment");
            HANDLE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "Array bounds exceeded");
            HANDLE(EXCEPTION_FLT_DENORMAL_OPERAND, "Floating point NaN operand");
            HANDLE(EXCEPTION_FLT_DIVIDE_BY_ZERO, "Floating point division by zero");
            HANDLE(EXCEPTION_FLT_INEXACT_RESULT, "Floating point inexact result");
            HANDLE(EXCEPTION_FLT_INVALID_OPERATION, "Floating point invalid operation");
            HANDLE(EXCEPTION_FLT_OVERFLOW, "Floating point overflow");
            HANDLE(EXCEPTION_FLT_STACK_CHECK, "Floating point stack error");
            HANDLE(EXCEPTION_FLT_UNDERFLOW, "Floating point underflow");
            HANDLE(EXCEPTION_INT_DIVIDE_BY_ZERO, "Integer division by zero");
            HANDLE(EXCEPTION_INT_OVERFLOW, "Integer overflow");
            HANDLE(EXCEPTION_PRIV_INSTRUCTION, "Privileged instruction");
            HANDLE(EXCEPTION_IN_PAGE_ERROR, "In page error");
            HANDLE(EXCEPTION_ILLEGAL_INSTRUCTION, "Illegal instruction");
            HANDLE(EXCEPTION_NONCONTINUABLE_EXCEPTION, "Execution after noncontinuable exception");
            HANDLE(EXCEPTION_STACK_OVERFLOW, "Stack overflow");
            HANDLE(EXCEPTION_INVALID_DISPOSITION, "Invalid exception disposition");
            HANDLE(EXCEPTION_GUARD_PAGE, "Guard page accessed");
            HANDLE(EXCEPTION_INVALID_HANDLE, "Invalid handle");
            HANDLE(CONTROL_C_EXIT, "User-initiated break");
            #undef HANDLE

            default:
                stream
                    << "Unknown structured exception code "
                    << record.ExceptionCode
                    << ".";
                break;
        }
        return stream.str();
    }

    void Registrar::writeStackTraceFile()
    {
        // Only continue if stack seems to be not corrupted
        if (magic != SEH_MAGIC_NUMBER)
            return;
        // Special treatment for stack overflow (we have not enough stack to do something else)
        if (record.ExceptionCode == EXCEPTION_STACK_OVERFLOW)
            return;
        if (logpath == NULL)
            return;
        if (withinHandler)
            return;
        withinHandler = true;
        try {
            time_t rawtime;
            struct tm* timeinfo;
            time(&rawtime);
            timeinfo = localtime ( &rawtime );

            std::stringstream filename;
            filename << std::setfill('0')
                << logpath << "stack-trace-"
                << std::setw(4) << (timeinfo->tm_year + 1900) << "-"
                << std::setw(2) << (timeinfo->tm_mon + 1) << "-"
                << std::setw(2) << timeinfo->tm_mday << "-"
                << std::setw(2) << timeinfo->tm_hour << "-"
                << std::setw(2) << timeinfo->tm_min << "-"
                << std::setw(2) << timeinfo->tm_sec << "-"
                << std::setw(4) << counter
                << ".txt";
            ++counter;

            std::fstream stream(filename.str(), std::fstream::out);
            stream << getExceptionText() << std::endl;
            printPrettyStack(stream, debugfilename, &context);
            stream.close();
        } catch (std::exception& e) { /* Do nothing, it is getting worst only... */ }
        withinHandler = false;
    }

    exception::exception(const std::string& what_arg)
        : std::runtime_error(what_arg)
    { }
}
