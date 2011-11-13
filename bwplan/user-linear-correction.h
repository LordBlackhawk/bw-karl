#pragma once

#include "planing/plan.h"
#include "planing/linear-correction.h"
#include "user-res-types.h"
#include <limits>

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
		std::cout << "Starting Time: " << it.time() << "\n";
	
		ResIndexType ri = ResIndexType::template byClass<RLarva>();
		TimeType starttime = -1;
		int growth = 0;
		while (!it.beyond()) {
			const ResourcesType& res = it.getResources();
			int value     = res.template getInternalValue<RLarva>();
			int maxvalue  = 3 * (res.template get<RZergHatchery>() + res.template get<RZergLair>() + res.template get<RZergHive>()) * ri.getScaling();
			int curgrowth = res.template getGrowth<RLarva>();
			if (starttime >= 0)
				value -= curgrowth * (it.time() - starttime);
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
				std::cout << "\tvalue changed at: " << it.time() << "\n";
				if (starttime >= 0) {
					plan.addCorrection(CorrectionType(ri, TimeInterval(starttime, it.time()), -growth));
					starttime = -1;
					it.update();
				}
			}

			int dt = it.getNextTime()-it.time();
			if (starttime < 0)
				dt = std::min(dt, (maxvalue-value+curgrowth)/curgrowth);
			it.inc(dt);
		}
		if (starttime >= 0) {
			std::cout << "\t" << TimeInterval(starttime, std::numeric_limits<TimeType>::max()) << " added.\n";
			plan.addCorrection(CorrectionType(ri, TimeInterval(starttime, std::numeric_limits<TimeType>::max()), -growth));
		}
	}

	template <class RSupply>
	static void supplyCorrections(PlanType& plan, SituationType it)
	{
		const int maxSupply = 400;
		ResIndexType ri = ResIndexType::template byClass<RSupply>();
		while (!it.beyond()) {
			int value = it.getResources().template getExisting<RSupply>();
			if (value > maxSupply) {
				plan.addCorrection(CorrectionType(ri, TimeInterval(it.time()-1, it.time()), maxSupply-value));
				it.update();
				assert(it.getResources().template getExisting<RSupply>() == maxSupply);
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
