#pragma once

DEF_CHECKPOINT(CSendGasWorker)
DEF_CHECKPOINT(CReturnGasWorker)

DEF_CHECKPOINT(CSendWorkerToBuildingPlace)
DEF_CHECKPOINT(CBuildAddon)
DEF_CHECKPOINT(CBuildAddonFinished)
DEF_CHECKPOINT(CBuildBuilding)
DEF_CHECKPOINT(CBuildingFinished)
DEF_CHECKPOINT(CMorphUnit)
DEF_CHECKPOINT(CCombineUnit)
DEF_CHECKPOINT(CTrainUnit)
DEF_CHECKPOINT(CUnitFinished)

BEGIN_DEF_OPTYPE(OSendTerranGasWorker)
    Locks<1, RGeyserWorkingPlace>,
    Locks<1, RTerranWorker>,
    Prods<1, RTerranGasWorker>,
        CheckPoint<CSendGasWorker, 1>,
END_DEF_OPTYPE
BEGIN_DEF_OPTYPE(OReturnTerranGasWorker)
    Consums<1, RTerranGasWorker>,
    Unlocks<1, RTerranWorker>,
    Unlocks<1, RGeyserWorkingPlace>,
        CheckPoint<CReturnGasWorker, 1>,
END_DEF_OPTYPE

DEF_ASSOCIATION(OSendTerranGasWorker,   BWAPI::Race, BWAPI::Races::Terran)
DEF_ASSOCIATION(OReturnTerranGasWorker, BWAPI::Race, BWAPI::Races::Terran)

DEF_SIMPLEREQUIREMENT(RGas,      OSendTerranGasWorker)
DEF_SIMPLEREQUIREMENT(RMinerals, OReturnTerranGasWorker)

BEGIN_DEF_OPTYPE(OSendProtossGasWorker)
    Locks<1, RGeyserWorkingPlace>,
    Locks<1, RProtossWorker>,
    Prods<1, RProtossGasWorker>,
        CheckPoint<CSendGasWorker, 1>,
END_DEF_OPTYPE
BEGIN_DEF_OPTYPE(OReturnProtossGasWorker)
    Consums<1, RProtossGasWorker>,
    Unlocks<1, RProtossWorker>,
    Unlocks<1, RGeyserWorkingPlace>,
        CheckPoint<CReturnGasWorker, 1>,
END_DEF_OPTYPE

DEF_ASSOCIATION(OSendProtossGasWorker,   BWAPI::Race, BWAPI::Races::Protoss)
DEF_ASSOCIATION(OReturnProtossGasWorker, BWAPI::Race, BWAPI::Races::Protoss)

DEF_SIMPLEREQUIREMENT(RGas,      OSendProtossGasWorker)
DEF_SIMPLEREQUIREMENT(RMinerals, OReturnProtossGasWorker)

BEGIN_DEF_OPTYPE(OSendZergGasWorker)
    Locks<1, RGeyserWorkingPlace>,
    Locks<1, RZergWorker>,
    Prods<1, RZergGasWorker>,
        CheckPoint<CSendGasWorker, 1>,
END_DEF_OPTYPE
BEGIN_DEF_OPTYPE(OReturnZergGasWorker)
    Consums<1, RZergGasWorker>,
    Unlocks<1, RZergWorker>,
    Unlocks<1, RGeyserWorkingPlace>,
        CheckPoint<CReturnGasWorker, 1>,
END_DEF_OPTYPE

DEF_ASSOCIATION(OSendZergGasWorker,   BWAPI::Race, BWAPI::Races::Zerg)
DEF_ASSOCIATION(OReturnZergGasWorker, BWAPI::Race, BWAPI::Races::Zerg)

DEF_SIMPLEREQUIREMENT(RGas,      OSendZergGasWorker)
DEF_SIMPLEREQUIREMENT(RMinerals, OReturnZergGasWorker)

typedef TL::type_list<
                    OSendTerranGasWorker, OReturnTerranGasWorker,
                    OSendProtossGasWorker, OReturnProtossGasWorker,
                    OSendZergGasWorker, OReturnZergGasWorker
                 > UserOperationTypeList;

