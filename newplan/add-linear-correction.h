#pragma once

#include "plan.h"

namespace {

typedef PlanContainer::Situation SituationType;

void larvaCorrections(PlanContainer& plan, SituationType it)
{
	std::cout << "Starting Time: " << it.time() << "\n";

	ResourceIndex ri(ResourceIndex::Larva);
	TimeType starttime = -1;
	int growth = 0;
	while (!it.beyond()) {
		const Resources& res = it.getResources();
		int value     = res.getInternal(ri);
		int maxvalue  = 3 * (res.get(ResourceIndex::ZergHatchery) + res.get(ResourceIndex::ZergLair) + res.get(ResourceIndex::ZergHive)) * ri.getScaling();
		int curgrowth = res.getGrowth(ri);
		if (starttime >= 0)
			value -= curgrowth * (it.time() - starttime);
		if (value > maxvalue) {
			if (starttime < 0) {
				starttime = it.time();
				growth    = curgrowth;
			} else if (growth != curgrowth) {
				plan.addCorrection(LinearCorrection(ri, TimeInterval(starttime, it.time()), -growth));
				starttime = it.time();
				growth    = curgrowth;
				it.update();
			}
		} else {
			std::cout << "\tvalue changed at: " << it.time() << "\n";
			if (starttime >= 0) {
				plan.addCorrection(LinearCorrection(ri, TimeInterval(starttime, it.time()), -growth));
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
		plan.addCorrection(LinearCorrection(ri, TimeInterval(starttime, std::numeric_limits<TimeType>::max()), -growth));
	}
}

void supplyCorrections(PlanContainer& plan, SituationType it, const ResourceIndex& ri)
{
	const int maxSupply = 400;
	while (!it.beyond()) {
		int value = it.getResources().getExisting(ri);
		if (value > maxSupply) {
			plan.addCorrection(LinearCorrection(ri, TimeInterval(it.time()-1, it.time()), maxSupply-value));
			it.update();
			assert(it.getResources().getExisting(ri) == maxSupply);
		}
		++it;
	}
}

} // end namespace

void PlanContainer::addCorrections(const TimeType& starttime)
{
	// 1. Larva Correction:
	larvaCorrections(*this, at(starttime));

	// 2. SupplyCorrection
	supplyCorrections(*this, at(starttime), ResourceIndex::TerranSupply);
	supplyCorrections(*this, at(starttime), ResourceIndex::ProtossSupply);
	supplyCorrections(*this, at(starttime), ResourceIndex::ZergSupply);
}