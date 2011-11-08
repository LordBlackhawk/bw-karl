#pragma once

#include "planing/plan.h"
#include "planing/linear-correction.h"
#include "user-res-types.h"

template <class Traits>
struct UserLinearCorrectionHandler
{
	typedef PlanContainer<Traits>			PlanType;
	typedef Resources<Traits>				ResourcesType;
	typedef ResourceIndex<Traits>			ResIndexType;
	typedef LinearCorrection<Traits>		CorrectionType;
	typedef typename PlanType::Situation	SituationType;

	static void larvaCorrections(PlanType& plan, SituationType it)
	{
		ResIndexType ri = ResIndexType::byClass<RLarva>();
		TimeType starttime = -1;
		int growth = 0;
		while (!it.beyond()) {
			const ResourcesType& res = it.getResources();
			int value     = res.getInternalValue<RLarva>();
			int maxvalue  = 3 * (res.get<RZergHatchery>() + res.get<RZergLair>() + res.get<RZergHive>()) * ri.getScaling();
			int curgrowth = res.getGrowth<RLarva>();
			if (value > maxvalue) {
				if (starttime < 0) {
					starttime = it.time();
					growth    = curgrowth;
				} else if (growth != curgrowth) {
					plan.addCorrection(CorrectionType(ri, TimeInterval(starttime, it.time()), -growth));
					starttime = it.time();
					growth    = curgrowth;
					it.update();
				}
			} else {
				if (starttime >= 0) {
					plan.addCorrection(CorrectionType(ri, TimeInterval(starttime, it.time()), -growth));
					stattime = -1;
					it.update();
				}
			}

			int dt = std::min((maxvalue-value+curgrowth)/curgrowth, it.getNextTime()-it.time());
			it.inc(dt);
		}
		if (starttime >= 0)
			plan.addCorrection(CorrectionType(ri, TimeInterval(starttime, MAXINT), -growth));
	}

	template <class RSupply>
	static void supplyCorrections(PlanType& plan, SituationType it)
	{
		const maxSupply = 400;
		ResIndexType ri = ResIndexType::byClass<RSupply>();
		while (!it.beyond()) {
			int value = it.getResources().getExisting<RSupply>();
			if (value > maxSupply) {
				plan.addCorrection(CorrectionType(ri, TimeInterval(it.time()-1, it.time()), maxSupply-value));
				it.update();
				assert(it.getResources().getExisting<RSupply>() == maxSupply);
			}
			++it;
		}
	}

	static void addCorrections(PlanType& plan, const SituationType& it)
	{
		// 1. Larva Correction:
		larvaCorrections(plan, it);

		// 2. SupplyCorrection
		supplyCorrections<RTerranSupply>(plan, it);
		supplyCorrections<RProtossSupply>(plan, it);
		supplyCorrections<RZergSupply>(plan, it);
	}
};
