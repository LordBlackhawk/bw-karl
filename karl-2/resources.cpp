// ToDo:
//  * Estimate after getResources can be improved.

#include "resources.hpp"
#include "mineral-line.hpp"
#include "vector-helper.hpp"
#include "utils/debug.h"
#include <algorithm>

using namespace BWAPI;

namespace
{
	struct ResourcesPreconditionInternal;
	
	std::vector<ResourcesPreconditionInternal*> reslist;
	int sum_m = 0;
	int sum_g = 0;

	struct ResourcesPreconditionInternal : public ResourcesPrecondition
	{
		ResourcesPreconditionInternal(int t, int m, int g)
			: ResourcesPrecondition(t, m, g)
		{
			sum_m += minerals;
			sum_g += gas;
			reslist.push_back(this);
		}

		virtual ~ResourcesPreconditionInternal()
		{
			VectorHelper::remove(reslist, this);
			sum_m -= minerals;
			sum_g -= gas;
		}
		
		int sortTime() const
		{
			return std::max(time, wishtime);
		}
	};
	
	struct ResourcesSorter
	{
		bool operator () (ResourcesPreconditionInternal* lhs, ResourcesPreconditionInternal* rhs) const
		{
			return lhs->sortTime() < rhs->sortTime();
		}
	};
}

ResourcesPrecondition* getResources(int m, int g)
{
	Player* self = Broodwar->self();
	int cur_m = self->minerals();
	int cur_g = self->gas();
	
	if (reslist.empty() && (m <= cur_m) && (g <= cur_g))
		return new ResourcesPreconditionInternal(0, m, g);
	
	int t = reslist.back()->time + 100;
	return new ResourcesPreconditionInternal(t, m, g);
}

ResourcesPrecondition* getResources(const BWAPI::UnitType& ut)
{
	return getResources(ut.mineralPrice(), ut.gasPrice());
}

void ResourcesCode::onMatchEnd()
{
	reslist.clear();
	sum_m = 0;
	sum_g = 0;
}

void ResourcesCode::onTick()
{
	std::stable_sort(reslist.begin(), reslist.end(), ResourcesSorter());

	Player* self = Broodwar->self();
	int cur_m = self->minerals();
	int cur_g = self->gas();
	
	auto it    = reslist.begin();
	auto itend = reslist.end();
	while ((it != itend) && ((*it)->minerals <= cur_m) && ((*it)->gas <= cur_g)) {
		(*it)->time = 0;
		cur_m -= (*it)->minerals;
		cur_g -= (*it)->gas;
		++it;
	}
	
	auto pit    = estimatedProduction.begin();
	auto pitend = estimatedProduction.end();
	int prod_m  = pit->minerals;
	int prod_g  = pit->gas;
	int time    = Broodwar->getFrameCount();
	++pit;
	cur_m *= MineralFactor;
	cur_g *= MineralFactor;
	
	for (; it!=itend; ++it) {
		int esttime;
		bool cont = true;
		while (cont) {
			int esttime_m = time + std::max(0, (MineralFactor * (*it)->minerals - cur_m) / prod_m);
			int esttime_g = time + std::max(0, (MineralFactor * (*it)->gas      - cur_g) / prod_g);
			esttime = std::max(esttime_m, esttime_g);
			
			cont = (pit != pitend) && (esttime > pit->time);
			if (cont) {
				int dt = pit->time - time;
				cur_m += dt * prod_m;
				cur_g += dt * prod_g;
				time  += dt;
				prod_m = pit->minerals;
				prod_g = pit->gas;
				++pit;
			}
		}
		
		(*it)->time = esttime;
		int dt = esttime - time;
		cur_m += dt * prod_m - MineralFactor * (*it)->minerals;
		cur_g += dt * prod_g - MineralFactor * (*it)->gas;
		time  += dt;
	}
}
