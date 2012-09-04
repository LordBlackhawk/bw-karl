#pragma once

#include "precondition.hpp"
#include <BWAPI.h>

struct ResourcesPrecondition : public Precondition
{
    int minerals;
    int gas;
	
	ResourcesPrecondition(int t, int m, int g)
		: Precondition(t), minerals(m), gas(g)
	{ }
};

struct SupplyPrecondition : public Precondition
{
	BWAPI::Race 	race;
	int 			supply;
	
	SupplyPrecondition(const BWAPI::Race& r, int s)
		: race(r), supply(s)
	{ }
};

struct UnitPrecondition : public Precondition
{
    BWAPI::UnitType     ut;
    BWAPI::Position     pos;
    BWAPI::Unit*        unit;

    UnitPrecondition(const BWAPI::UnitType& t, const BWAPI::Position& p, BWAPI::Unit* u)
        : Precondition(0), ut(t), pos(p), unit(u)
    { }
	
	UnitPrecondition(int time, const BWAPI::UnitType& t, const BWAPI::Position& p)
        : Precondition(time), ut(t), pos(p), unit(NULL)
    { }
};
