#include "myseh-internal.hpp"
#include <iostream>
#include <map>

namespace
{
    class PrettyStackPrinter
    {
        const char* debugfilename;
        std::map<void*, std::string> names;
        void* minStack;
        void* maxStack;

        void addName(void* addr, const std::string& name)
        {
            std::map<void*, std::string>::iterator it = names.find(addr);
            if (it != names.end()) {
                it->second = it->second + "  " + name;
            } else {
                names[addr] = name;
            }

            if (addr < minStack)
                minStack = addr;
            if (addr > maxStack)
                maxStack = addr;
        }

        void addNameOfCode(void* addr, const std::string& name)
        {
            if (debugfilename != NULL) {
                std::string location = readDebugInfo(debugfilename, *(void**)addr);
                if (location != "") {
                    addName(addr, "[" + location + "] " + name);
                    return;
                }
            }
            addName(addr, name);
        }

        std::string lookupName(void* addr) const
        {
            std::map<void*, std::string>::const_iterator it = names.find(addr);
            if (it != names.end())
                return it->second;
            return "";
        }

        void markExceptionHandler(void* addr)
        {
            addName(addr, "<-- Exception frame");
            addNameOfCode(((unsigned int*)addr)+1, "<-- Exception handler");
        }

        void markAllExceptionHandlers(void* addr)
        {
            unsigned int value;
            while ((value = (unsigned int)addr, (value != 0) && (value != 1) && (value != 0xFFFFFFFF))) {
                markExceptionHandler(addr);
                addr = *(void**)addr;
            }
        }

        void markStackFrame(void* addr)
        {
            addName(addr, "<-- Stack frame");
            addNameOfCode(((unsigned int*)addr)+1, "<-- Return address");
        }

        void markAllStackFrames(void* addr)
        {
            unsigned int value;
            while ((value = (unsigned int)addr, (value != 0) && (value != 1) && (value != 0xFFFFFFFF))) {
                markStackFrame(addr);
                addr = *(void**)addr;
            }
        }

        public:
            void printOutput(std::ostream& stream) const
            {
                const char* linesep = "**************************************************\n";

                stream
                    << std::hex
                    << std::setfill('0')
                    << "Stack trace:\n"
                    << linesep;

                for(unsigned int* addr=(unsigned int*)minStack; addr<=maxStack; ++addr)
                {
                    unsigned int value = *(unsigned int*)addr;
                    stream
                        << std::setw(8) << (unsigned)addr
                        << " : "
                        << std::setw(8) << value
                        << " "
                        << lookupName(addr)
                        << "\n";
                }

                stream
                    << linesep;
            }

            PrettyStackPrinter(const char* d, void* Ebp, void* Esp)
                : debugfilename(d), minStack(Esp), maxStack(Esp)
            {
                markAllExceptionHandlers(*(void**)__libseh_get_registration());
                markAllStackFrames(Ebp);
                addName(Esp, "<-- Esp");
                addName(Ebp, "<-- Ebp");
            }
    };
}

namespace seh
{
    void printPrettyStack(std::ostream& stream, const char* debugfilename, const CONTEXT* pContext)
    {
        void* Ebp;
        void* Esp;
        if (pContext != NULL) {
            Ebp = (void*)pContext->Ebp;
            Esp = (void*)pContext->Esp;
        } else {
            asm volatile ("movl %%ebp, %0" : "=r" (Ebp));
            asm volatile ("movl %%esp, %0" : "=r" (Esp));
        }
        PrettyStackPrinter(debugfilename, Ebp, Esp).printOutput(stream);
    }
}
