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
			std::pair<ChokepointInfoPtr, ChokepointInfoPtr> pair = (c1 < c2) ? std::make_pair(c1, c2) : std::make_pair(c2, c1);
			auto it = distances.find(pair);
			if (it == distances.end())
				return 0.0;
			
			return it->second;
		}
	
	protected:
		BWTA::Region* region;
		
		BWAPI::Position center;
		std::set<ChokepointInfoPtr> chokepoints;
		std::map<std::pair<ChokepointInfoPtr, ChokepointInfoPtr>, double> distances;
		
		void init();
		
		RegionInfo(BWTA::Region* r) : region(r)
		{
			center = region->getCenter();
		}
};
