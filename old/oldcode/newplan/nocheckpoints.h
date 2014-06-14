#pragma once

#include "checkpoints.h"

CheckPointResult::type CSendGasWorker(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CReturnGasWorker(Operation& /*op*/)  { return CheckPointResult::completed; }

CheckPointResult::type CSendWorkerToBuildingPlace(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CBuildAddon(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CBuildAddonFinished(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CBuildBuilding(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CBuildingFinished(Operation& /*op*/)  { return CheckPointResult::completed; }

CheckPointResult::type CMorphUnit(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CCombineUnit(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CTrainUnit(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CUnitFinished(Operation& /*op*/)  { return CheckPointResult::completed; }

CheckPointResult::type CTechStart(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CTechFinished(Operation& /*op*/)  { return CheckPointResult::completed; }

CheckPointResult::type CUpgradeStart(Operation& /*op*/)  { return CheckPointResult::completed; }
CheckPointResult::type CUpgradeFinished(Operation& /*op*/)  { return CheckPointResult::completed; }