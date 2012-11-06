#include "bwapi-helper.hpp"

using namespace BWAPI;

BWAPI::Unit* getNearest(const std::set<BWAPI::Unit*>& units, const BWAPI::Position& p)
{
	if (units.empty())
		return NULL;
	
	auto it = units.begin();
	auto itend = units.end();
	Unit* result = *it;
	double dis = p.getDistance((*it)->getPosition());
	++it;
	
	while (it != itend)
	{
		double newdis = p.getDistance((*it)->getPosition());
		if (newdis < dis) {
			result = *it;
			dis    = newdis;
		}
		++it;
	}
	
	return result;
}
