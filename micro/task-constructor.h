#pragma once

#include "micro-task.h"
#include <BWAPI.h>

MicroTask createLongMove(const BWAPI::Position& target);

MicroTask createBuild(const BWAPI::UnitType& ut, const BWAPI::TilePosition& target);
MicroTask createBuildObserver();

MicroTask createMorph(const BWAPI::UnitType& ut);
MicroTask createMorphObserver();

MicroTask createTrain(const BWAPI::UnitType& ut);
MicroTask createTrainObserver();

MicroTask createTech(const BWAPI::TechType& tt);
MicroTask createTechObserver();

MicroTask createUpgrade(const BWAPI::UpgradeType& gt);
MicroTask createUpgradeObserver();
