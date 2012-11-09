#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"
#include "unit-micromanagement.hpp"


class Squad
{
public:
    BWAPI::Position getCenter();
    
    int getUnitCount(const BWAPI::UnitType& ut);
    
    void defend(BWAPI::Position pos);
    
    //void addUnit(UnitMicromanagement *unit);
    void addUnit(BWAPI::Unit *unit);
    void addUnit(MicromanagedUnit *microUnit);
    
    void setName(std::string desc);
    
    Squad();
	virtual ~Squad();
    
    virtual void onTick();
    virtual void onDrawPlan();
    
private:
    BWAPI::Position defendposition;
    //std::set<UnitMicromanagement *> units;
    std::string name;
    std::set<MicromanagedUnit*> units;
};

struct SquadCode : public DefaultCode
{
    static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onDrawPlan();
	static void onCheckMemoryLeaks();
};
