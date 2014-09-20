#pragma once

#include "plan/plan-item.hpp"
#include "utils/join-strings.hpp"

namespace boost
{
    namespace program_options
    {
        class options_description;
    }
}

class ExpertRegistrar
{
    public:
        typedef bool (*IsApplicableFunc) (Blackboard*);
        typedef AbstractExpert* (*CreateInstanceFunc) ();

        ExpertRegistrar(const char* name, IsApplicableFunc isApplicable, CreateInstanceFunc createInstance);

        static void prepareBlackboard(Blackboard* blackboard);
};

#define REGISTER_EXPERT(name)                                                                                               \
    namespace { AbstractExpert* MY_JOIN(createInstanceOf, name)() { return new name(); }                                    \
                ExpertRegistrar MY_JOIN(register, name)(#name, name::isApplicable, &MY_JOIN(createInstanceOf, name)); }
