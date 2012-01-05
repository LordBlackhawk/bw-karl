#pragma once

#include <BWTA.h>

#include <set>

class RegionInfo
{
	friend class InformationKeeper;
	
	public:
		BWTA::Region* get() const
		{
			return region;
		}
		
		BWAPI::Position getCenter() const
		{
			return center;
		}
		
		const std::set<ChokepointInfoPtr>& getChokepoints() const
		{
			return chokepoints;
		}
		
		double getGroundDistance(ChokepointInfoPtr c1, ChokepointInfoPtr c2) const
		{
			std::pair<ChokepointInfoPtr, ChokepointInfoPtr> pair = (*it < *iit) ? std::make_pair(*it, *iit) : std::make_pair(*iit, *it);
			auto it = distances.find(pair);
			if (it == distances.end())
				return 0.0;
			
			return it.second;
		}
	
	protected:
		BWTA::Region* region;
		
		BWAPI::Position center;
		std::set<ChokepointInfoPtr> chokepoints;
		std::map<std::pair<ChokepointInfoPtr, ChokepointInfoPtr>, double> distances;
		
		void init()
		{
			for (auto it : region->getChokepoints())
				chokepoints.insert(InformationKeeper::instance().getInfo(it));
			
			auto it    = chokepoints.begin();
			auto itend = chokepoints.end();
			for (; it!=itend; ++it)
				for (auto iit = it, ++iit; iit!=itend; ++iit)
			{
				std::pair<ChokepointInfoPtr, ChokepointInfoPtr> pair = (*it < *iit) ? std::make_pair(*it, *iit) : std::make_pair(*iit, *it);
				double distance = BWTA::getGroundDistance((*it)->getPosition(), (*iit)->getPosition());
				distances[pair] = distance;
			}
		}
		
		RegionInfo(BWTA::Region* r) : region(r)
		{
			center = region->getCenter();
		}
};
