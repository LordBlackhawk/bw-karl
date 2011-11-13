#pragma once

#include "timetype.h"
#include "operationstatus.h"

#include <boost/shared_ptr.hpp>

struct CheckPointResult
{
	enum type { waiting, running, completed, failed };
};

struct Operation;

struct SendWorkerDetails;
void CSendGasWorker(Operation& op);
void CReturnGasWorker(Operation& op);

struct BuildBuildingDetails;
void CSendWorkerToBuildingPlace(Operation& op);
void CBuildAddon(Operation& op);
void CBuildAddonFinished(Operation& op);
void CBuildBuilding(Operation& op);
void CBuildingFinished(Operation& op);

struct BuildUnitDetails;
void CMorphUnit(Operation& op);
void CCombineUnit(Operation& op);
void CTrainUnit(Operation& op);
void CUnitFinished(Operation& op);

struct TechDetails;
void CTechStart(Operation& op);
void CTechFinished(Operation& op);

struct UpgradeDetails;
