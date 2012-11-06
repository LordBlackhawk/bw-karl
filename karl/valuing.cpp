#include "valuing.hpp"
#include "precondition.hpp"

#include <BWAPI.h>

#include <algorithm>

using namespace BWAPI;

// used in mineral-line.cpp for minimization.
// Important: Value must be positive.
double valueWorkerAssignment(bool isPlanedWorker, bool isGasJob, bool isMineralJob, int /*time*/, int /*wishtime*/, 
                             const BWAPI::Position& pos, const BWAPI::Position& wishpos, bool assigned)
{
    double result = 0.0;
    bool isExternal = !(isGasJob || isMineralJob);
    
    if (isPlanedWorker && isExternal)
        result += 10000.0;
    
    if (isExternal)
        result += pos.getDistance(wishpos);
        
    if (!assigned)
        result += 100.0;
    
    return result;
}

double valueWorkerAssignmentNoJob(int time)
{
    return (time == 0) ? 1000.0 : 0.0;
}

double valueWorkerAssignmentNoAgent(bool isGasJob, bool isMineralJob, int wishtime)
{
    double result = 0.0;
    bool isExternal = !(isGasJob || isMineralJob);
    
    if (isGasJob)
        result += 100.0;

    if (isExternal)
        result += 10000.0;
    
    //result += std::fabs(Precondition::Impossible - wishtime);
    
    return result;
}

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
