// ToDo:
//  * Add setGasMode(None / Auto)

#include "resources.hpp"
#include "mineral-line.hpp"
#include "vector-helper.hpp"
#include "valuing.hpp"
#include "utils/debug.h"
#include <algorithm>

using namespace BWAPI;

namespace
{
	struct ResourcesPreconditionInternal;
	
	std::vector<ResourcesPreconditionInternal*> reslist;
	int indexcounter = 0;

	struct ResourcesPreconditionInternal : public ResourcesPrecondition
	{
		int index;
		
		ResourcesPreconditionInternal(int m, int g)
			: ResourcesPrecondition(Precondition::Impossible, m, g)
		{
			reslist.push_back(this);
			index = ++indexcounter;
		}

		virtual ~ResourcesPreconditionInternal()
		{
			VectorHelper::remove(reslist, this);
		}
		
		double sortValue() const
		{
			return valueResources(time, wishtime, index);
		}
	};
	
	struct ResourcesSorter
	{
		bool operator () (ResourcesPreconditionInternal* lhs, ResourcesPreconditionInternal* rhs) const
		{
			return lhs->sortValue() < rhs->sortValue();
		}
	};
}

ResourcesPrecondition* getResources(int m, int g)
{
	return new ResourcesPreconditionInternal(m, g);
}

ResourcesPrecondition* getResources(const BWAPI::UnitType& ut)
{
	return getResources(ut.mineralPrice(), ut.gasPrice());
}

void ResourcesCode::onMatchBegin()
{
	indexcounter = 0;
}

void ResourcesCode::onMatchEnd()
{
	reslist.clear();
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
