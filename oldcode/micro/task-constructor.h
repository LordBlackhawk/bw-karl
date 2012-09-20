#pragma once

#include "informations/informations.h"
#include "micro-task.h"
#include <BWAPI.h>

MicroTaskPtr createLongMove(const BWAPI::Position& target);
MicroTaskPtr createRegionMove(const BWAPI::Position& target);
MicroTaskPtr createChokepointMove(ChokepointInfoPtr point, RegionInfoPtr region);

MicroTaskPtr createBuild(const BWAPI::UnitType& ut, const BWAPI::TilePosition& target);
MicroTaskPtr createBuildObserver();

MicroTaskPtr createMorph(const BWAPI::UnitType& ut);
MicroTaskPtr createMorphObserver();

MicroTaskPtr createTrain(const BWAPI::UnitType& ut);
MicroTaskPtr createTrainObserver();

MicroTaskPtr createTech(const BWAPI::TechType& tt);
MicroTaskPtr createTechObserver();

MicroTaskPtr createUpgrade(const BWAPI::UpgradeType& gt);
MicroTaskPtr createUpgradeObserver();
