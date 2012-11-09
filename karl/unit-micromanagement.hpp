#pragma once

#include "default-code.hpp"

class Squad;

struct MicromanagedUnit;
struct UnitMicromanagementCode;

#define NOW (BWAPI::Broodwar->getFrameCount()+1)
#define NEXTTHINK (BWAPI::Broodwar->getFrameCount()+BWAPI::Broodwar->getLatencyFrames()+1)
#define LATER (BWAPI::Broodwar->getFrameCount()+24)

//int MICROMANAGEMENTFUNC_MODEL(MicromanagedUnit*,void*){return 0;}
//typedef decltype(&MICROMANAGEMENTFUNC_MODEL) MICROMANAGEMENTFUNC;

//typedef decltype([](MicromanagedUnit*,void*){return 0;}) MICROMANAGEMENTFUNC;

struct MicromanagedUnit
{
    enum GoalType { wait, moveToPosition, moveToTarget, attackPosition, attackTarget, unknown };
    enum StatusType { idle, moving, attacking, runningAway };
    
    MicromanagedUnit();
    MicromanagedUnit(BWAPI::Unit *u,int (*(*funcs)[GoalType::unknown])(MicromanagedUnit*,void*));

    static MicromanagedUnit* getMicromanagement(BWAPI::Unit *u);
    ~MicromanagedUnit();
    
    inline BWAPI::Unit *getUnit(){return unit;}
    inline BWAPI::UnitType getType(){return unit->getType();}
    inline BWAPI::Position getPosition(){return unit->getPosition();}
    inline bool exists(){return unit->exists();}
    
    inline bool isIdle(){return status==StatusType::idle;}
    
    inline GoalType getGoal(){return goal;}
    inline void setGoal(GoalType g){goal=g;nextThink=NOW;}
    
    inline StatusType getStatus(){return status;}
    inline void attack(BWAPI::Position pos){targetPosition=pos;goal=GoalType::attackPosition;nextThink=NOW;}
    inline void attack(BWAPI::Unit *unit){target=unit;goal=GoalType::attackTarget;nextThink=NOW;}
    inline void follow(BWAPI::Unit *unit){target=unit;goal=GoalType::moveToTarget;nextThink=NOW;}
    inline void move(BWAPI::Position pos){targetPosition=pos;goal=GoalType::moveToPosition;nextThink=NOW;}

    inline void setCourage(float c){if(c<0)courage=0;else if(c>1)courage=1; else courage=c;}
    inline float getCourage(){return courage;}
    
    inline void setTarget(BWAPI::Unit *t){target=t;nextThink=NOW;}
    inline BWAPI::Unit * getTarget(){return target;}
    
    inline void setTargetPosition(BWAPI::Position p){targetPosition=p;nextThink=NOW;}
    inline BWAPI::Position getTargetPosition(){return targetPosition;}
    
    inline bool canDo(GoalType goal){return goal<GoalType::unknown?(*funcs)[goal]!=NULL:false;}
    
    friend struct UnitMicromanagementCode;
    
protected:
    BWAPI::Unit *unit;
    StatusType status;
    GoalType goal;
    
    int nextThink;
    
    void *data[GoalType::unknown];
    int (*(*funcs)[GoalType::unknown])(MicromanagedUnit*,void*);
    
    float courage;
    BWAPI::Position targetPosition;
    BWAPI::Unit *target;
    
    
    inline void setStatus(StatusType stat){status=stat;} //internal use in micromanagement funcs only
    
};

struct UnitMicromanagementCode : public DefaultCode
{
    static void onMatchBegin();
	static void onMatchEnd();
	//static void onUnitCreate(BWAPI::Unit* unit);
	//static void onUnitDestroy(BWAPI::Unit* unit);
	static void onTick();
	static void onDrawPlan(HUDTextOutput& hud);
	static void onCheckMemoryLeaks();
};


MicromanagedUnit *micromanageUnit(BWAPI::Unit *unit);
