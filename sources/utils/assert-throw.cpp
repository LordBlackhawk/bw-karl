#include "assert-throw.hpp"
#include <sstream>

AssertException::AssertException(const std::string& what_arg)
    : std::runtime_error(what_arg)
{ }

void AssertException::throwInstance(const char* file, int line, const char* txt)
{
    std::stringstream stream;
    stream << "Assertion '" << txt << "' failed at '" << file << ":" << line << "'.";
    throw AssertException(stream.str());
}
