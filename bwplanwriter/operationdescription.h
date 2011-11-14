#pragma once

#include "variables.h"
#include "resourcedescription.h"
#include <BWAPI.h>
#include <string>
#include <vector>


struct ItemDescription
{
	enum Type { Needs, Locks, Unlocks, Consums, Prods, CheckPoint };
	Type type;
	int count;
	ResourceDescription* res;
	std::string name;
	
	ItemDescription(std::string n, int duration)
		: type(CheckPoint), count(duration), name(n)
	{ }
	
	ItemDescription(Type t, int c, ResourceDescription* r) : type(t), count(c), res(r)
	{ }
};

struct OperationDescription
{
	std::string name;
	std::vector<ItemDescription> items;
	
	int stagecount;
	int duration;
	
	BWAPI::Race race;
	BWAPI::UnitType ut;
	BWAPI::TechType tt;
	BWAPI::UpgradeType gt;
	
	OperationDescription(std::string n)
		: name(n), race(BWAPI::Races::None), ut(BWAPI::UnitTypes::None), tt(BWAPI::TechTypes::None), gt(BWAPI::UpgradeTypes::None)
	{
		operationDescriptions.push_back(this);
	}
	
	void calculate()
	{
		stagecount = 0;
		duration   = 0;
		for (auto it : items)
			if (it.type == ItemDescription::CheckPoint) {
				++stagecount;
				duration += it.count;
			}
	}
	
	void add(ItemDescription i)
	{
		items.push_back(i);
	}
	
	void checkpoint(std::string n, int duration)
	{
		add(ItemDescription(n, duration));
	}
	
	void needs(int c, ResourceDescription* r)
	{
		add(ItemDescription(ItemDescription::Needs, c, r));
	}
	
	void consums(int c, ResourceDescription* r)
	{
		add(ItemDescription(ItemDescription::Consums, c, r));
	}
	
	void locks(int c, ResourceDescription* r)
	{
		add(ItemDescription(ItemDescription::Locks, c, r));
	}
	
	void unlocks(int c, ResourceDescription* r)
	{
		add(ItemDescription(ItemDescription::Unlocks, c, r));
	}
	
	void prods(int c, ResourceDescription* r)
	{
		add(ItemDescription(ItemDescription::Prods, c, r));
	}
};
