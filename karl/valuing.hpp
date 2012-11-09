#pragma once

#include "default-code.hpp"
#include <BWTA.h>

typedef int ctype;

ctype valueWorkerAssignment(int time, int wishtime,
                            const BWAPI::Race& race, const BWAPI::Race& wishrace,
                            const BWAPI::Position& pos, const BWAPI::Position& wishpos,
                            bool isPlanedWorker, bool isGasJob, bool isMineralJob, bool isWorkerIdle, bool assigned);
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
