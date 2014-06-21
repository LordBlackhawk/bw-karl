#include "expert-registrar.hpp"
#include "secure-expert.hpp"
#include <algorithm>

namespace
{
    struct ExpertStruct
    {
        ExpertRegistrar::IsApplicableFunc isApplicable;
        ExpertRegistrar::CreateInstanceFunc createInstance;

        ExpertStruct(ExpertRegistrar::IsApplicableFunc f1, ExpertRegistrar::CreateInstanceFunc f2)
            : isApplicable(f1), createInstance(f2)
        { }
    };

    std::map<std::string, ExpertStruct>& instance()
    {
        static std::map<std::string, ExpertStruct> inst;
        return inst;
    }
}

ExpertRegistrar::ExpertRegistrar(const char* name, IsApplicableFunc isApplicable, CreateInstanceFunc createInstance)
{
    instance().insert(std::make_pair(std::string(name), ExpertStruct(isApplicable, createInstance)));
}

void ExpertRegistrar::preapreBlackboard(Blackboard* blackboard)
{
    for (auto it : instance()) {
        if (it.second.isApplicable(blackboard))
            blackboard->addExpert(new SecureExpert(it.first, it.second.createInstance()));
    }
}
