#pragma once

#include "variables.h"

#include <string>
#include <vector>
#include <utility>

struct ResourceDescription
{
	std::string name;
	bool lockable;
	int scaling;
	std::vector< std::pair<int, ResourceDescription*> > growth, influence;
	std::vector< OperationDescription* > associated;
	
	BWAPI::Race race;
	BWAPI::UnitType ut;
	BWAPI::TechType tt;
	BWAPI::UpgradeType gt;
	
	ResourceDescription(std::string n, bool l = false, int s = 1)
		: name(n), lockable(l), scaling(s), race(BWAPI::Races::None), ut(BWAPI::UnitTypes::None), tt(BWAPI::TechTypes::None), gt(BWAPI::UgradeTypes::None)
	{
		resourceDescriptions.push_back(this);
	}
	
	void addGrowth(int multiplier, ResourceDescription* base)
	{
		growth.push_back(std::make_pair(multiplier, base));
		base->influence.push_back(std::make_pair(multiplier, this));
	}
};
