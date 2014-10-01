#include "expert-registrar.hpp"
#include "secure-expert.hpp"
#include "utils/log.hpp"
#include "utils/options.hpp"
#include <algorithm>

namespace
{
    struct ExpertStruct
    {
        ExpertRegistrar::IsApplicableFunc   isApplicable;
        ExpertRegistrar::CreateInstanceFunc createInstance;
        bool                                disabled;

        ExpertStruct(bool enabled, ExpertRegistrar::IsApplicableFunc f1, ExpertRegistrar::CreateInstanceFunc f2)
            : isApplicable(f1), createInstance(f2), disabled(!enabled)
        { }
    };

    std::map<std::string, ExpertStruct>& instance()
    {
        static std::map<std::string, ExpertStruct> inst;
        return inst;
    }

    bool optSecureExpert = false;
    std::vector<std::string> disabledExperts;
    std::string onlyExpert;

    AbstractExpert* secureExpert(const std::string& name, AbstractExpert* expert)
    {
        return optSecureExpert ? new SecureExpert(name, expert) : expert;
    }
}

ExpertRegistrar::ExpertRegistrar(const char* name, bool enabled, IsApplicableFunc isApplicable, CreateInstanceFunc createInstance)
{
    instance().insert(std::make_pair(std::string(name), ExpertStruct(enabled, isApplicable, createInstance)));
}

void ExpertRegistrar::prepareBlackboard(Blackboard* blackboard)
{
    for (auto it : instance()) {
        if (!it.second.disabled && it.second.isApplicable(blackboard))
            blackboard->addExpert(secureExpert(it.first, it.second.createInstance()));
    }
}

DEF_OPTIONS
{
    po::options_description options("Expert options");
    options.add_options()
            ("secure",      po::bool_switch(&optSecureExpert),          "Catch exceptions experts are throwing.")
            ("disable",     po::value< std::vector<std::string> >(&disabledExperts),        "Disable an expert.")
            ("only",        po::value<std::string>(&onlyExpert),                "Run only the specified expert.")
        ;
    return options;
}

DEF_OPTION_EVENT(onEvaluate)
{
    auto& map = instance();
    bool error = false;

    // read 'only'-option
    if (!onlyExpert.empty()) {
        bool found = false;
        for (auto& it : map) {
            if (it.first == onlyExpert) {
                found = true;
                it.second.disabled = false;
            } else if(it.first!="WebGUIExpert") {
                it.second.disabled = true; //disable all other experts, except WebGUI
            }
        }
        if (!found) {
            std::cout << "ERROR: Unknown expert '" << onlyExpert << "'.\n";
            error = true;
        }
        if (!disabledExperts.empty()) {
            std::cout << "ERROR: The option 'only' should not be combined with 'disable'.\n";
            error = true;
        }
    }

    // read 'disable'-option
    for (auto name : disabledExperts) {
        auto it = map.find(name);
        if (it == map.end()) {
            std::cout << "ERROR: Unknown expert '" << name << "'.\n";
            error = true;
            continue;
        }
        it->second.disabled = true;
    }

    disabledExperts.clear();
    onlyExpert.clear();
    if (error)
        exit(1);
}

DEF_OPTION_EVENT(onHelp)
{
    std::cout << "\nExpert names:\n\n";
    auto& map = instance();
    for (auto it : map) {
        std::cout << "  " << it.first << "\n";
    }
}
