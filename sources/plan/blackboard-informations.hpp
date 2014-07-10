#pragma once

#include "utils/time.hpp"
#include <BWAPI.h>
#include <BWTA.h>

class MineralBoundaryItem;

struct BaseLocation
{
    BWTA::BaseLocation*             origin;
    std::set<MineralBoundaryItem*>  minerals;
};

struct BlackboardInformations
{
    ~BlackboardInformations();

    Time    lastUpdateTime = -1;
    int     currentMinerals = 0;
    int     currentGas = 0;

    BWAPI::Player* self = NULL;

    std::set<BaseLocation*>     allBaseLocations;
    std::set<BaseLocation*>     ownBaseLocations;
};
