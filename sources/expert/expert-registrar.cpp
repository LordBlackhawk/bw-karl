#include "expert-registrar.hpp"
#include "secure-expert.hpp"
#include "utils/log.hpp"
#include <boost/program_options.hpp>
#include <algorithm>

namespace po = boost::program_options;

namespace
{
    struct ExpertStruct
    {
        ExpertRegistrar::IsApplicableFunc   isApplicable;
        ExpertRegistrar::CreateInstanceFunc createInstance;
        bool                                disabled;

        ExpertStruct(ExpertRegistrar::IsApplicableFunc f1, ExpertRegistrar::CreateInstanceFunc f2)
            : isApplicable(f1), createInstance(f2), disabled(false)
        { }
    };

    std::map<std::string, ExpertStruct>& instance()
    {
        static std::map<std::string, ExpertStruct> inst;
        return inst;
    }

    bool optSecureExpert = false;
    std::vector<std::string> disabledExperts;

    AbstractExpert* secureExpert(const std::string& name, AbstractExpert* expert)
    {
        return optSecureExpert ? new SecureExpert(name, expert) : expert;
    }
}

ExpertRegistrar::ExpertRegistrar(const char* name, IsApplicableFunc isApplicable, CreateInstanceFunc createInstance)
{
    instance().insert(std::make_pair(std::string(name), ExpertStruct(isApplicable, createInstance)));
}

void ExpertRegistrar::prepareBlackboard(Blackboard* blackboard)
{
    for (auto it : instance()) {
        if (!it.second.disabled && it.second.isApplicable(blackboard))
            blackboard->addExpert(secureExpert(it.first, it.second.createInstance()));
    }
}

boost::program_options::options_description ExpertRegistrar::getOptions()
{
    po::options_description options("Expert options");
    options.add_options()
            ("secure",      po::bool_switch(&optSecureExpert),          "Catch exceptions experts are throwing.")
            ("disable",     po::value< std::vector<std::string> >(&disabledExperts),        "Disable an expert.")
        ;
    return options;
}

void ExpertRegistrar::evaluateOptions()
{
    auto& map = instance();
    for (auto name : disabledExperts) {
        auto it = map.find(name);
        if (it == map.end()) {
            std::cout << "ERROR: Unknown expert '" << name << "'.\n";
            continue;
        }
        it->second.disabled = true;
    }
    disabledExperts.clear();
}
