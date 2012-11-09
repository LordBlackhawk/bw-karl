#pragma once

#include "default-code.hpp"

class Squad;

class UnitMicromanagement
{
    
public:
    UnitMicromanagement(BWAPI::Unit *unit):unit(unit){}
    virtual ~UnitMicromanagement(){}
    
    virtual void onTick(){}
    virtual void onDrawPlan(){}
    
    
    virtual void attack(BWAPI::Position pos);
    virtual void defend(BWAPI::Position pos);
    virtual bool isIdle();
    
    
    inline BWAPI::Position getPosition(){return unit->getPosition(); }
    inline BWAPI::UnitType getType(){return unit->getType(); }
    inline BWAPI::UnitType exists(){return unit->exists(); }
    inline BWAPI::Unit *getUnit(){return unit; }
    
private:
    friend class Squad;
    void _assignToSquad(Squad *squad); //for Squad
    
protected:
    BWAPI::Unit *unit;
    Squad *squad;
};

class UnitMicromanagementMarine : public UnitMicromanagement
{
public:
    UnitMicromanagementMarine(BWAPI::Unit *unit) : UnitMicromanagement(unit){}
    virtual ~UnitMicromanagementMarine(){}
    
    virtual void onTick();
    virtual void onDrawPlan();
    
protected:
};

class UnitMicromanagementMedic : public UnitMicromanagement
{
public:
    UnitMicromanagementMedic(BWAPI::Unit *unit) : UnitMicromanagement(unit){}
    virtual ~UnitMicromanagementMedic(){}
    
    virtual void onTick();
    virtual void onDrawPlan();
    
protected:
};

class UnitMicromanagementFirebat : public UnitMicromanagement
{
public:
    UnitMicromanagementFirebat(BWAPI::Unit *unit) : UnitMicromanagement(unit){}
    virtual ~UnitMicromanagementFirebat(){}
    
    virtual void onTick();
    virtual void onDrawPlan();
    
protected:
};


struct UnitMicromanagementCode : public DefaultCode
{
    static void onMatchBegin();
	static void onMatchEnd();
	static void onUnitCreate(BWAPI::Unit* unit);
	static void onUnitDestroy(BWAPI::Unit* unit);
	static void onTick();
	static void onDrawPlan(HUDTextOutput& hud);
	static void onCheckMemoryLeaks();
};
