#pragma once

#include "default-code.hpp"
#include <BWTA.h>

typedef int ctype;

ctype valueWorkerAssignment(bool isPlanedWorker, bool isGasJob, bool isMineralJob, bool isWorkerIdle, int time, int wishtime, 
                             const BWAPI::Position& pos, const BWAPI::Position& wishpos, bool assigned);
ctype valueWorkerAssignmentNoJob(int time);
ctype valueWorkerAssignmentNoAgent(bool isGasJob, bool isMineralJob, int wishtime);
ctype valueLarvaAssignment(int time, int wishtime, int index, bool assigned);
ctype valueResources(int time, int wishtime, int index);
ctype valueSupply(int time, int wishtime, int index);
ctype valueExpo(BWTA::BaseLocation* expo, BWTA::BaseLocation* home);

struct ValuingCode : public DefaultCode
{
    static void onTick();
};
