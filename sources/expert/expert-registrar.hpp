#pragma once

#include "plan/plan-item.hpp"
#include "utils/join-strings.hpp"

class ExpertRegistrar
{
    public:
        typedef bool (*IsApplicableFunc) (Blackboard*);
        typedef AbstractExpert* (*CreateInstanceFunc) ();

        ExpertRegistrar(const char* name, bool enabled, IsApplicableFunc isApplicable, CreateInstanceFunc createInstance);

        static void prepareBlackboard(Blackboard* blackboard);
};

#define REGISTER_EXPERT_EX(name, enabled)                                                                                       \
    namespace { AbstractExpert* MY_JOIN(createInstanceOf, name)() { return new name(); }                                        \
                ExpertRegistrar MY_JOIN(register, name)(#name, enabled, name::isApplicable, &MY_JOIN(createInstanceOf, name)); }

#define REGISTER_EXPERT(name)   REGISTER_EXPERT_EX(name, true)
