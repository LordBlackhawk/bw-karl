#pragma once

#include "plan.h"

namespace {

typedef PlanContainer::Situation SituationType;

void larvaCorrections(PlanContainer& plan, SituationType it)
{
	ResourceIndex ri(ResourceIndex::Larva);
	TimeType starttime = -1;
	int growth = 0;
	for (; it!=plan.end(); ++it)
	{
		const Resources& res = it.getResources();
		int value     = res.getInternal(ri);
		int maxvalue  = 3 * (res.get(ResourceIndex::ZergHatchery) + res.get(ResourceIndex::ZergLair) + res.get(ResourceIndex::ZergHive)) * ri.getScaling();
		int curgrowth = res.getGrowth(ri);
		if (starttime >= 0) {
			value -= curgrowth * (it.time() - starttime);
			if (value < maxvalue) {
				plan.addCorrection(LinearCorrection(ri, TimeInterval(starttime, it.time()), -growth));
				starttime = -1;
				it.update();
			} else if (growth != curgrowth) {
				plan.addCorrection(LinearCorrection(ri, TimeInterval(starttime, it.time()), -growth));
				starttime = it.time();
				growth    = curgrowth;
				it.update();
			}
		} else if (curgrowth > 0) {
			TimeType maxreached = it.time() + (maxvalue-value+curgrowth-1)/curgrowth;
			if (maxreached < it.getNextTime()) {
				starttime = maxreached;
				growth    = curgrowth;
			}
		}
	}
	if (starttime >= 0)
		plan.addCorrection(LinearCorrection(ri, TimeInterval(starttime, std::numeric_limits<TimeType>::max()), -growth));
}

void supplyCorrections(PlanContainer& plan, SituationType it, const ResourceIndex& ri)
{
	const int maxSupply = 400;
	for (; it!=plan.end(); ++it)
	{
		int value = it.getResources().getExisting(ri);
		if (value > maxSupply) {
			plan.addCorrection(LinearCorrection(ri, TimeInterval(it.time()-1, it.time()), maxSupply-value));
			it.update();
			assert(it.getResources().getExisting(ri) == maxSupply);
		}
	}
}

} // end namespace

void PlanContainer::addCorrections(const TimeType& starttime)
{
	//std::clog << "begin addCorrections\n";
	// 1. Larva Correction:
	larvaCorrections(*this, at(starttime, true));

	// 2. SupplyCorrection
	supplyCorrections(*this, at(starttime, true), ResourceIndex::TerranSupply);
	supplyCorrections(*this, at(starttime, true), ResourceIndex::ProtossSupply);
	supplyCorrections(*this, at(starttime, true), ResourceIndex::ZergSupply);
	//std::clog << "end addCorrections\n";
}