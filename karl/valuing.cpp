#include "valuing.hpp"
#include "precondition.hpp"

#include <BWAPI.h>

#include <algorithm>

using namespace BWAPI;

namespace
{
    bool isStrictTick = false;
}

void ValuingCode::onTick()
{
    //int frame = Broodwar->getFrameCount();
    //isStrictTick = (frame % 100 < 6);
}

// used in mineral-line.cpp for minimization.
// Important: Value must be positive.
ctype valueWorkerAssignment(int time, int wishtime,
                            const BWAPI::Race& race, const BWAPI::Race& wishrace,
                            const BWAPI::Position& pos, const BWAPI::Position& wishpos,
                            bool isPlanedWorker, bool isGasJob, bool isMineralJob, bool isWorkerIdle, bool assigned)
{
    ctype result = 0;
    bool isExternal = !(isGasJob || isMineralJob);
    
    if (isPlanedWorker && isExternal && (wishtime < time))
        result += 10000;
    
    if (isPlanedWorker && isGasJob)
        result += 200;

    if (isWorkerIdle && isExternal)
        result += 15000;
    
    if (wishrace != Races::Unknown)
        if (race != wishrace)
            result += 20000;
        
    if (!assigned)
        result += 100;
    
    if ((pos != Positions::Unknown) && (wishpos != Positions::Unknown)) {
        double dis = pos.getDistance(wishpos);
        result += std::max(99, int(dis/32));
    }
    
    return result;
}

ctype valueWorkerAssignmentNoJob(int time)
{
    return (time == 0) ? 700 : 107;
}

ctype valueWorkerAssignmentNoAgent(bool isGasJob, bool isMineralJob, int /*wishtime*/)
{
    ctype result = 0;
    bool isExternal = !(isGasJob || isMineralJob);
    
    if (isMineralJob)
        result += 3;
    
    if (isGasJob)
        result += 3000;

    if (isExternal)
        result += 30000;
    
    return result;
}

// used in larvas.cpp for minimization.
// Important: Value must be positive.
ctype valueLarvaAssignment(int time, int wishtime, int index, bool assigned)
{
    if (isStrictTick)
        return time * index;

    ctype result = 0;
    
    if (!assigned)
        result += 100;

    if (time == 0) {
        int cur = Broodwar->getFrameCount();
        if (wishtime > cur + 50)
            result += 2 * (wishtime - cur);
    } else if (wishtime < time) {
        result += 100;
    }

    return result;
}

// used in resources.cpp for minimization.
ctype valueResources(int time, int wishtime, int index)
{
    if (isStrictTick)
        return index;

    return 100 * index + std::max(time, wishtime);
}

// used in supply.cpp for minimization.
ctype valueSupply(int time, int wishtime, int index)
{
    if (isStrictTick)
        return index;

    return 100 * index + std::max(time, wishtime);
}

// used in building-placer.cpp for maximization.
ctype valueExpo(BWTA::BaseLocation* expo, BWTA::BaseLocation* home)
{
    return - 10 * (ctype) home->getGroundDistance(expo)
           - 3 * (ctype) home->getAirDistance(expo)
           + (expo->isMineralOnly() ? 0 : 3000)
           + 500 * expo->getStaticMinerals().size()
           ;
}
