#include "string-helper.hpp"
#include <sstream>
#include <iterator>

std::vector<std::string> splitIntoWords(const std::string& text)
{
    std::istringstream stream(text);
    std::vector<std::string> words((std::istream_iterator<std::string>(stream)), 
                                   std::istream_iterator<std::string>());
    return words;
}

bool startsWith(const std::string& txt, const std::string& sub)
{
    return txt.substr(0, sub.size()) == sub;
}
