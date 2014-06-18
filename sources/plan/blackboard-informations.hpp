#pragma once

#include "utils/time.hpp"
#include <BWAPI.h>

struct BlackboardInformations
{
    Time    lastUpdateTime = -1;
    int     currentMinerals = 0;
    int     currentGas = 0;

    BWAPI::Player* self = NULL;
};
