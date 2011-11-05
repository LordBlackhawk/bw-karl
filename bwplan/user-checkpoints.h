#pragma once

DEF_CHECKPOINT(CSendGasWorker)
DEF_CHECKPOINT(CReturnGasWorker)

struct BuildBuildingDetails;
DEF_CHECKPOINT(CSendWorkerToBuildingPlace)
DEF_CHECKPOINT(CBuildAddon)
DEF_CHECKPOINT(CBuildAddonFinished)
DEF_CHECKPOINT(CBuildBuilding)
DEF_CHECKPOINT(CBuildingFinished)

struct BuildUnitDetails;
DEF_CHECKPOINT(CMorphUnit)
DEF_CHECKPOINT(CCombineUnit)
DEF_CHECKPOINT(CTrainUnit)
DEF_CHECKPOINT(CUnitFinished)

struct TechDetails;
DEF_CHECKPOINT(CTechStart)
DEF_CHECKPOINT(CTechFinished)

struct UpgradeDetails;
