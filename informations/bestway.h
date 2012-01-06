#pragma once

#include "information.h"

#include <queue>

RegionInfoPtr InformationKeeper::getRegion(const BWAPI::Position& pos) const
{
	return getInfo(BWTA::getRegion(TilePosition(pos)));
}

BaseLocationInfoPtr InformationKeeper::getNearestBaseLocation(const BWAPI::Position& pos) const
{
	return getInfo(BWTA::getNearestBaseLocation(pos));
}

ChokepointInfoPtr InformationKeeper::getNearestChokepoint(const BWAPI::Position& pos) const
{
	return getInfo(BWTA::getNearestChokepoint(pos));
}

struct BestWayHelper
{
	double distance;
	CheckpointInfoPtr first;
	RegionInfoPtr from;
	CheckpointInfoPtr current;
	
	BestWayHelper(RegionInfoPtr r, CheckpointInfoPtr cp, double dis) : distance(dis), first(cp), region(r), current(cp)
	{ }
	
	bool operator < (const BestWayHelper& other) const
	{
		return (distance < other.distance);
	}
};

template <class FUNCTIONAL>
double InformationKeeper::getBestWay(const FUNCTIONAL& f, const BWAPI::Position& pos, const BWAPI::Position& target, ChokepointInfoPtr& next)
{	
	RegionInfoPtr region = getRegion(start);
	RegionInfoPtr endregion = getRegion(target);
	
	std::priority_queue<BestWayHelper> list;
	for (auto it : region.getChokepoints())
		if (f.useChokepoint(it))
			list.push(BestWayHelper(region, it, BWTA::getGroundDistance(start, it->getPosition()) + f.malusChokepoint(it)));
	
	std::set<ChokepointInfoPtr> visited;
	while (!list.empty())
	{
		BestWayHelper cur = list.top(); list.pop();
		
		ChokepointInfoPtr point = cur.current;
		if (point.use_count() == 0) {
			next = cur.first;
			return cur.distance;
		}
		
		if (visited.find(point) != visited.end())
			continue;
		visited.insert(point);
		
		RegionInfoPtr lastregion = cur.form;
		RegionInfoPtr newregion = point->getOtherRegion(lastregion);
		if (newregion == endregion) {
			cur.distance += BWTA::getGroundDistance(point->getPosition(), target);
			cur.current = ChokepointInfoPtr();
			list.push(cur);
		} else {
			cur.from = newregion;
			double dis = cur.distance + f.malusRegion(newregion);
			for (auto it : newregion.getChokepoints())
				if (it != point)
					if (f.useChokepoint(it))
			{
				cur.current = it;
				cur.distance = dis + newregion.getGroundDistance(point, it) + f.malusChokepoint(it);
				list.push(cur);
			}
		}
	}
	
	return -1.0;
}

struct ShortestWayFunctional
{
	bool useChokepoint(ChokepointInfoPtr point) const
	{
		return true;
	}
	
	double malusChokepoint(ChokepointInfoPtr point) const
	{
		return 0.0;
	}
	
	double malusRegion(RegionInfoPtr region) const
	{
		return 0.0;
	}
};

double getShortestWay(const BWAPI::Position& pos, const BWAPI::Position& target, ChokepointInfoPtr& next)
{
	ShortestWayFunctional f;
	return getBestWay(f, pos, target, next);
}