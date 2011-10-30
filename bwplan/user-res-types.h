#pragma once

DEF_RESTYPE(RMinerals)
DEF_RESTYPE(RGas)
DEF_RESTYPE(RGeyserWorkingPlace)

DEF_RESTYPE(RTerranWorker)
DEF_RESTYPE(RTerranGasWorker)
DEF_RESTYPE(RTerranSupply)
DEF_RESTYPE(RProtossWorker)
DEF_RESTYPE(RProtossGasWorker)
DEF_RESTYPE(RProtossSupply)
DEF_RESTYPE(RZergWorker)
DEF_RESTYPE(RZergGasWorker)
DEF_RESTYPE(RZergSupply)

DEF_RESTYPE(RLarva)
// TODO: Growth for RLarva

DEF_RESLOCKABLE(RGeyserWorkingPlace)
DEF_RESLOCKABLE(RTerranWorker)
DEF_RESLOCKABLE(RTerranSupply)
DEF_RESLOCKABLE(RProtossWorker)
DEF_RESLOCKABLE(RProtossSupply)
DEF_RESLOCKABLE(RZergWorker)
DEF_RESLOCKABLE(RZergSupply)

#define MINERALVALUE 45
BEGIN_DEF_RESGROWTH(RMinerals, 1000)
	LINEAR(MINERALVALUE, RTerranWorker),
	LINEAR(MINERALVALUE, RProtossWorker),
	LINEAR(MINERALVALUE, RZergWorker)
END_DEF_RESGROWTH

#define GASVALUE 45
BEGIN_DEF_RESGROWTH(RGas, 1000)
	LINEAR(GASVALUE, RTerranGasWorker),
	LINEAR(GASVALUE, RProtossGasWorker),
	LINEAR(GASVALUE, RZergGasWorker)
END_DEF_RESGROWTH

// BEGIN_DEF_RESGROWTH(RLarva, 1000)
// END_DEF_RESGROWTH

typedef TL::type_list<
                    RMinerals, RGas, RGeyserWorkingPlace,
                    RTerranWorker, RTerranGasWorker, RTerranSupply,
                    RProtossWorker, RProtossGasWorker, RProtossSupply,
                    RLarva, RZergWorker, RZergGasWorker, RZergSupply
                 > UserResourceTypeList;

