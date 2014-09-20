#pragma once

#include <boost/program_options.hpp>
#include "utils/join-strings.hpp"

namespace po = boost::program_options;

class OptionsRegistrar
{
    public:
        enum Event { onHelp, onEvaluate };

        typedef po::options_description (*OptionFunction) ();
        typedef void (*OptionEventFunction) ();

        OptionsRegistrar(OptionFunction f);
        OptionsRegistrar(Event e, OptionEventFunction f);

        static void insertAllOptions(po::options_description& desc);
        static void evaluateOptions(int argc, char* argv[]);

        static bool optLog();
        static bool optParallel();
        static bool optHUD();
        static int  optSpeed();
};

#define DEF_OPTIONS_INTERNAL(nameRegistrar, nameFunction)           \
    po::options_description nameFunction();                         \
    OptionsRegistrar nameRegistrar(&nameFunction);                  \
    po::options_description nameFunction()

#define DEF_OPTIONS \
    DEF_OPTIONS_INTERNAL(MY_JOIN(registerOptions, __LINE__), MY_JOIN(functionOptions, __LINE__))

#define DEF_OPTION_EVENT_INTERNAL(event, nameRegistrar, nameFunction)       \
    void nameFunction();                                                    \
    OptionsRegistrar nameRegistrar(event, &nameFunction);                   \
    void nameFunction()

#define DEF_OPTION_EVENT(event) \
    DEF_OPTION_EVENT_INTERNAL(OptionsRegistrar::event, MY_JOIN(registerOptionEvent, __LINE__), MY_JOIN(functionOptionEvent, __LINE__))
