#pragma once

#include <BWTA.h>

double valueWorkerAssignment(bool isPlanedWorker, bool isGasJob, bool isMineralJob, int time, int wishtime, 
                             const BWAPI::Position& pos, const BWAPI::Position& wishpos, bool assigned);
double valueWorkerAssignmentNoJob(int time);
double valueWorkerAssignmentNoAgent(bool isGasJob, bool isMineralJob, int wishtime);
double valueLarvaAssignment(int time, int wishtime, bool assigned);
double valueResources(int time, int wishtime, int index);
double valueExpo(BWTA::BaseLocation* expo, BWTA::BaseLocation* home);
