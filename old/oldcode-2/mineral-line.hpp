#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

struct Production
{
    int time;       // starting time.
    int minerals;   // in 1/MineralFactor mineral per frame.
    int gas;        // in 1/MineralFactor gas per frame.
};

const int MineralFactor = 1000;

extern std::vector<Production> estimatedProduction; // sorted by time.

void useWorker(BWAPI::Unit* unit);
void useWorker(UnitPrecondition* unit);
UnitPrecondition* getWorker(const BWAPI::Race& r);
void registerBase(BWAPI::Unit* u);
UnitPrecondition* registerBase(UnitPrecondition* b);

bool buildRefinery(const BWAPI::UnitType& type);

void setGasWorkerPerRefinery(int nw = 3);
int freeMineralLinePlaces();

struct MineralLineCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onCheckMemoryLeak();
};
