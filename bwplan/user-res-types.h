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

DEF_RESLOCKABLE(RGeyserWorkingPlace)
DEF_RESLOCKABLE(RTerranWorker)
DEF_RESLOCKABLE(RTerranSupply)
DEF_RESLOCKABLE(RProtossWorker)
DEF_RESLOCKABLE(RProtossSupply)
DEF_RESLOCKABLE(RZergWorker)
DEF_RESLOCKABLE(RZergSupply)

#define MINERALVALUE 45
DEF_RESGROWTH(RMinerals, MINERALVALUE, RTerranWorker)
DEF_RESGROWTH(RMinerals, MINERALVALUE, RProtossWorker)
DEF_RESGROWTH(RMinerals, MINERALVALUE, RZergWorker)

#define GASVALUE 45
DEF_RESGROWTH(RGas, GASVALUE, RTerranGasWorker)
DEF_RESGROWTH(RGas, GASVALUE, RProtossGasWorker)
DEF_RESGROWTH(RGas, GASVALUE, RZergGasWorker)

typedef type_list<
                    RMinerals, RGas, RGeyserWorkingPlace,
                    RTerranWorker, RTerranGasWorker, RTerranSupply,
                    RProtossWorker, RProtossGasWorker, RProtossSupply,
                    RZergWorker, RZergGasWorker, RZergSupply
                 > BWUserResourceList;

