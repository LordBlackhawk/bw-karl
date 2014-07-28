#pragma once

#include "plan/plan-item.hpp"

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
        static boost::program_options::options_description getOptions();
        static void evaluateOptions();
        static void listExperts(std::ostream& stream);
};

#define MY_JOIN( X, Y ) MY_DO_JOIN( X, Y )
#define MY_DO_JOIN( X, Y ) MY_DO_JOIN2(X,Y)
#define MY_DO_JOIN2( X, Y ) X##Y

#define REGISTER_EXPERT(name)                                                                                               \
    namespace { AbstractExpert* MY_JOIN(createInstanceOf, name)() { return new name(); }                                    \
                ExpertRegistrar MY_JOIN(register, name)(#name, name::isApplicable, &MY_JOIN(createInstanceOf, name)); }
