#ifndef UNITAGENTMAP_h
#define UNITAGENTMAP_h

#include "agent.h"
#include <map>

class UnitAgentMap
{
public:
	typedef std::map<BWAPI::Unit*, Agent*> MapType;

	static void add(BWAPI::Unit* unit, Agent* agent)
	{
		getInstance().map[unit] = agent;
	}

	static Agent* get(BWAPI::Unit* unit)
	{
		MapType::iterator it = getInstance().map.find(unit);
		if (it != getInstance().map.end())
			return it->second;
		else
			return NULL;
	}

	static void remove(BWAPI::Unit* unit)
	{
		getInstance().map.erase(unit);
	}

private:
	UnitAgentMap()
	{ }

	MapType map;

	static UnitAgentMap& getInstance();
};

#endif