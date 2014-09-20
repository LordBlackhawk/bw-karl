#include "options.hpp"
#include <iostream>

namespace
{
    struct EventStruct
    {
        OptionsRegistrar::Event                 event;
        OptionsRegistrar::OptionEventFunction   fun;
        EventStruct(OptionsRegistrar::Event e, OptionsRegistrar::OptionEventFunction f)
            : event(e), fun(f)
        { }
    };

    std::vector<OptionsRegistrar::OptionFunction>& getFunctions()
    {
        static std::vector<OptionsRegistrar::OptionFunction> instance;
        return instance;
    }

    std::vector<EventStruct>& getEventFunctions()
    {
        static std::vector<EventStruct> instance;
        return instance;
    }
    
    void callEvent(OptionsRegistrar::Event event)
    {
        for (auto it : getEventFunctions())
            if (it.event == event)
                it.fun();
    }
}

OptionsRegistrar::OptionsRegistrar(OptionFunction f)
{
    getFunctions().push_back(f);
}

OptionsRegistrar::OptionsRegistrar(Event e, OptionEventFunction f)
{
    getEventFunctions().push_back(EventStruct(e, f));
}

void OptionsRegistrar::insertAllOptions(po::options_description& desc)
{
    for (auto it : getFunctions())
        desc.add(it());
}

namespace
{
    bool writelogfiles = false;
    bool showhud = false;
    bool doParallel = false;
    int speed = 0;
}

void OptionsRegistrar::evaluateOptions(int argc, char* argv[])
{
    bool showhelp = false;

    po::options_description all("All options");

    po::options_description general("General options");
    general.add_options()
            ("help",        po::bool_switch(&showhelp),                 "Show this help message.")
            ("log",         po::bool_switch(&writelogfiles),            "Write log files on windows exceptions.")
            ("hud",         po::bool_switch(&showhud),                  "Show HUD.")
            ("speed",       po::value<int>(&speed)->default_value(0),   "Set game speed (-1 = default, 0 maximum speed, ...)")
            ("parallel",    po::bool_switch(&doParallel),               "Run experts parallel to StarCraft.")
        ;
    all.add(general);
    insertAllOptions(all);

    try {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(all).run(), vm);
        po::notify(vm);
    } catch (po::error& e) {
        std::cout << e.what() << "\n\n" << all;
        exit(1);
    }

    if (showhelp) {
        std::cout << all;
        callEvent(onHelp);
        exit(0);
    }

    callEvent(onEvaluate);
}

bool OptionsRegistrar::optLog()
{
    return writelogfiles;
}

bool OptionsRegistrar::optParallel()
{
    return doParallel;
}

bool OptionsRegistrar::optHUD()
{
    return showhud;
}

int OptionsRegistrar::optSpeed()
{
    return speed;
}
