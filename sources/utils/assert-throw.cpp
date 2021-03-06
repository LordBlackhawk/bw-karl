#include "assert-throw.hpp"
#include <sstream>

void AssertBase::writeDebugInformations(std::ostream& /*stream*/)
{ }

void AssertBase::throwAssertException(const char* file, int line, const char* txt)
{
    std::stringstream stream;
    stream << "Assertion '" << txt << "' failed at '" << file << ":" << line << "'.\n";
    writeDebugInformations(stream);
    throw AssertException(stream.str());
}

AssertException::AssertException(const std::string& what_arg)
    : std::runtime_error(what_arg)
{ }

void throwAssertException(const char* file, int line, const char* txt)
{
    std::stringstream stream;
    stream << "Assertion '" << txt << "' failed at '" << file << ":" << line << "'.";
    throw AssertException(stream.str());
}
