#include "secure-expert.hpp"
#include "utils/log.hpp"
#include "utils/myseh.hpp"

#include <exception>

SecureExpert::SecureExpert(const std::string& n, AbstractExpert* e)
    : name(n), expert(e)
{ }

SecureExpert::~SecureExpert()
{
    delete expert;
}

void SecureExpert::prepare()
{
    try {
        expert->prepare();
    } catch (std::exception& e) {
        LOG << "Expert '" << name << "' raised exception '" << e.what() << "' while 'prepare'.";
    }
}

bool SecureExpert::tick(Blackboard* blackboard)
{
    try {
        return expert->tick(blackboard);
    } catch (std::exception& e) {
        LOG << blackboard->getLastUpdateTime() << ": Expert '" << name << "' raised exception '" << e.what() << "' while 'tick'.";
    }
    return true;
}