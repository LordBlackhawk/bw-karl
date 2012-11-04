#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"


class Squad
{
public:
    BWAPI::Position getCenter();
    
    int getUnitCount(const BWAPI::UnitType& ut);
    
    void defend(BWAPI::Position pos);
    
    void addUnit(BWAPI::Unit *u);
    
    void setName(std::string desc);
    
    Squad();
    
	virtual ~Squad()
	{ }
    
    virtual void onTick();
    virtual void onDrawPlan();
    
private:
    BWAPI::Position defendposition;
    std::set<BWAPI::Unit*> units;
    std::string name;
    //std::multimap<BWAPI::UnitType,BWAPI::Unit*> unitmap;
};

struct SquadCode : public DefaultCode
{
    static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onDrawPlan();
	static void onCheckMemoryLeaks();
};
