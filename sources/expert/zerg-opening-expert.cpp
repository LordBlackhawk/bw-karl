#include "zerg-opening-expert.hpp"
#include "expert-registrar.hpp"
#include "utils/log.hpp"
#include "utils/options.hpp"

REGISTER_EXPERT(ZergOpeningExpert)

namespace
{
    enum OpeningType { open4Pool, open6PoolSunken, open9PoolSpeed, openRandom }; // Random has to be last element!

    bool play4Pool          = false;
    bool play6PoolSunken    = false;
    bool play9PoolSpeed     = false;
    OpeningType opening     = openRandom;
}

DEF_OPTIONS
{
    po::options_description options("Zerg opening options");
    options.add_options()
            ("4pool",           po::bool_switch(&play4Pool),          "Play always 4 pool.")
            ("6poolsunken",     po::bool_switch(&play6PoolSunken),    "Play always 5 pool + sunken.")
            ("9poolspeed",      po::bool_switch(&play9PoolSpeed),     "Play always 9 pool + speed.")
        ;
    return options;
}

DEF_OPTION_EVENT(onEvaluate)
{
    int numOptionsSet = 0;
    if (play4Pool) {
        opening = open4Pool;
        ++numOptionsSet;
    }
    if (play6PoolSunken) {
        opening = open6PoolSunken;
        ++numOptionsSet;
    }
    if (play9PoolSpeed) {
        opening = open9PoolSpeed;
        ++numOptionsSet;
    }
    if (numOptionsSet > 1) {
        std::cout << "ERROR: Multiple options for Zerg opening set, but only one possible.\n";
        exit(1);
    }
}

bool ZergOpeningExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

bool ZergOpeningExpert::tick(Blackboard* blackboard)
{
    OpeningType thisOpening = opening;
    if (thisOpening == openRandom)
        thisOpening = (OpeningType)(rand() % (int)openRandom);

    switch (thisOpening)
    {
        case open4Pool:
            LOG << "Opening 4 pool.";
            blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
            break;

        case open6PoolSunken:
            LOG << "Opening 6 pool + sunken.";
            for (int k=0; k<2; ++k)
                blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
            blackboard->create(BWAPI::UnitTypes::Zerg_Sunken_Colony);
            break;

        default:
        case open9PoolSpeed:
            LOG << "Opening 9 pool + speed.";
            for (int k=0; k<5; ++k)
                blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
            blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
            blackboard->build(BWAPI::UnitTypes::Zerg_Extractor);
            blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
            blackboard->morph(BWAPI::UnitTypes::Zerg_Overlord);
            blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
            blackboard->upgrade(BWAPI::UpgradeTypes::Metabolic_Boost);
            break;
    }

    return false;
}
