#include "valuing.hpp"

#include <BWAPI.h>

#include <algorithm>

using namespace BWAPI;

// used in larvas.cpp for minimization.
// Important: Value must be positive.
double valueLarvaAssignment(int time, int wishtime, bool assigned)
{
    double result = 0.0;
    
    if (!assigned)
        result += 100.0;

    if (time == 0) {
        int cur = Broodwar->getFrameCount();
        if (wishtime > cur + 50)
            result += 2.0 * (wishtime - cur);
    } else if (wishtime < time) {
        result += 100.0;
    }

    return result;
}

// used in resources.cpp for minimization.
double valueResources(int time, int wishtime, int index)
{
    return 1000.0 * index + std::max(time, wishtime);
}

// used in building-placer.cpp for minimization.
double valueExpo(BWTA::BaseLocation* expo, BWTA::BaseLocation* home)
{
    return - 1.0 * home->getGroundDistance(expo)
           - 0.3 * home->getAirDistance(expo)
           + (expo->isMineralOnly() ? 0 : 300.0)
           + 50.0 * expo->getStaticMinerals().size()
           ;
}
