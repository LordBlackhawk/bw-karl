#pragma once

#include "timetype.h"
#include "operationstatus.h"

struct CheckPointResult
{
	enum type { waiting, running, completed, failed };
};

struct Operation;

struct SendWorkerDetails;
CheckPointResult::type CSendGasWorker(Operation& op);
CheckPointResult::type CReturnGasWorker(Operation& op);

struct BuildBuildingDetails;
CheckPointResult::type CSendWorkerToBuildingPlace(Operation& op);
CheckPointResult::type CBuildAddon(Operation& op);
CheckPointResult::type CBuildAddonFinished(Operation& op);
CheckPointResult::type CBuildBuilding(Operation& op);
CheckPointResult::type CBuildingFinished(Operation& op);

struct BuildUnitDetails;
CheckPointResult::type CMorphUnit(Operation& op);
CheckPointResult::type CCombineUnit(Operation& op);
CheckPointResult::type CTrainUnit(Operation& op);
CheckPointResult::type CUnitFinished(Operation& op);

struct TechDetails;
CheckPointResult::type CTechStart(Operation& op);
CheckPointResult::type CTechFinished(Operation& op);

struct UpgradeDetails;
CheckPointResult::type CUpgradeStart(Operation& op);
CheckPointResult::type CUpgradeFinished(Operation& op);
