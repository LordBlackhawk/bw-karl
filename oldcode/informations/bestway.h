#pragma once

#include "informations.h"

#include <queue>

RegionInfoPtr InformationKeeper::getRegion(const BWAPI::Position& pos)
{
	return getInfo(BWTA::getRegion(BWAPI::TilePosition(pos)));
}

RegionInfoPtr InformationKeeper::getRegion(const BWAPI::TilePosition& tilepos)
{
	return getInfo(BWTA::getRegion(tilepos));
}

BaseLocationInfoPtr InformationKeeper::getNearestBaseLocation(const BWAPI::Position& pos)
{
	return getInfo(BWTA::getNearestBaseLocation(pos));
}

ChokepointInfoPtr InformationKeeper::getNearestChokepoint(const BWAPI::Position& pos)
{
	return getInfo(BWTA::getNearestChokepoint(pos));
}

struct BestWayHelper
{
	double distance;
	ChokepointInfoPtr first;
	RegionInfoPtr from;
	ChokepointInfoPtr current;
	
	BestWayHelper(RegionInfoPtr r, ChokepointInfoPtr cp, double dis) : distance(dis), first(cp), from(r), current(cp)
	{ }
	
	bool operator < (const BestWayHelper& other) const
	{
		return (distance < other.distance);
	}
};

template <class FUNCTIONAL>
double InformationKeeper::getBestWay(const FUNCTIONAL& f, const BWAPI::TilePosition& pos, const BWAPI::TilePosition& target, ChokepointInfoPtr& next)
{	
	RegionInfoPtr region = getRegion(pos);
	RegionInfoPtr endregion = getRegion(target);
	
	std::priority_queue<BestWayHelper> list;
	for (auto it : region->getChokepoints())
		if (f.useChokepoint(it))
			list.push(BestWayHelper(region, it, BWTA::getGroundDistance(pos, it->getTilePosition()) + f.malusChokepoint(it)));
	
	std::set<ChokepointInfoPtr> visited;
	while (!list.empty())
	{
		BestWayHelper cur = list.top(); list.pop();
		
		ChokepointInfoPtr point = cur.current;
		if (point == NULL) {
			next = cur.first;
			return cur.distance;
		}
		
		if (visited.find(point) != visited.end())
			continue;
		visited.insert(point);
		
		RegionInfoPtr lastregion = cur.from;
		RegionInfoPtr newregion = point->getOtherRegion(lastregion);
		if (newregion == endregion) {
			cur.distance += BWTA::getGroundDistance(point->getTilePosition(), target);
			cur.current = ChokepointInfoPtr();
			list.push(cur);
		} else {
			cur.from = newregion;
			double dis = cur.distance + f.malusRegion(newregion);
			for (auto it : newregion->getChokepoints())
				if (it != point)
					if (f.useChokepoint(it))
			{
				cur.current = it;
				cur.distance = dis + newregion->getGroundDistance(point, it) + f.malusChokepoint(it);
				list.push(cur);
			}
		}
	}
	
	return -1.0;
}

struct ShortestWayFunctional
{
	bool useChokepoint(ChokepointInfoPtr /*point*/) const
	{
		return true;
	}
	
	double malusChokepoint(ChokepointInfoPtr /*point*/) const
	{
		return 0.0;
	}
	
	double malusRegion(RegionInfoPtr /*region*/) const
	{
		return 0.0;
	}
};

double InformationKeeper::getShortestWay(const BWAPI::TilePosition& pos, const BWAPI::TilePosition& target, ChokepointInfoPtr& next)
{
	ShortestWayFunctional f;
	return getBestWay(f, pos, target, next);
}