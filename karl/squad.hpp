#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"


class UnitMicromanagement;

class Squad
{
public:
    BWAPI::Position getCenter();
    
    int getUnitCount(const BWAPI::UnitType& ut);
    
    void defend(BWAPI::Position pos);
    
    void addUnit(UnitMicromanagement *unit);
    void addUnit(BWAPI::Unit *unit);
    
    std::string getName() const;
    void setName(std::string desc);
    
    Squad();
	virtual ~Squad();
    
    virtual void onTick();
    virtual void onDrawPlan();
    
private:
    BWAPI::Position defendposition;
    std::set<UnitMicromanagement *> units;
    std::string name;
    //std::multimap<BWAPI::UnitType,BWAPI::Unit*> unitmap;
};

Squad* getSquadByName(const std::string& name);

struct SquadCode : public DefaultCode
{
    static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onDrawPlan(HUDTextOutput& hud);
	static void onCheckMemoryLeaks();
};
