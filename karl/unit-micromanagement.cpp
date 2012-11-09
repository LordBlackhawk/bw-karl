
// ToDo:
// *

#include "log.hpp"

#include "unit-micromanagement.hpp"
#include "squad.hpp"

#include <BWTA.h>


using namespace BWAPI;

namespace
{
    struct MicromanagementHelper : public virtual MicromanagedUnit
    {
    };
    struct MicromanagementHelperDefault : public MicromanagementHelper
    {
        int defaultWait()
        {
            if(!getUnit()->isIdle())
                getUnit()->stop();
            
            setStatus(MicromanagedUnit::StatusType::idle);

            return LATER;
        }
        int defaultMoveToTarget()
        {
            if(getStatus()!=MicromanagedUnit::StatusType::moving)
            {
                if(!getTarget())
                {
                    WARNING << "no target to move to!";
                    setGoal(MicromanagedUnit::GoalType::wait);
                    setStatus(MicromanagedUnit::StatusType::idle);
                    return LATER;
                }
                getUnit()->move(getTarget()->getPosition());
                setStatus(MicromanagedUnit::StatusType::moving);
                return NEXTTHINK;
            }
            else
            {
                if(getUnit()->isIdle())
                {
                    setStatus(MicromanagedUnit::StatusType::idle);
                }
            }

            return LATER;
        }
        int defaultMoveToPosition()
        {
            if(getStatus()!=MicromanagedUnit::StatusType::moving || getTargetPosition().getDistance(getUnit()->getTargetPosition())>32*10)
            {
                if(getTargetPosition()==Positions::None||getTargetPosition()==Positions::Invalid||getTargetPosition()==Positions::Unknown)
                {
                    WARNING << "no position to move to!";
                    setGoal(MicromanagedUnit::GoalType::wait);
                    setStatus(MicromanagedUnit::StatusType::idle);
                    return LATER;
                }
                getUnit()->move(getTargetPosition());
                setStatus(MicromanagedUnit::StatusType::moving);
                return NEXTTHINK;
            }
            else
            {
                if(getUnit()->isIdle())
                {
                    setStatus(MicromanagedUnit::StatusType::idle);
                }
            }
            return LATER;            
        }
        int defaultAttackTarget()
        {
            if(getStatus()!=MicromanagedUnit::StatusType::moving)
            {
                if(!getTarget())
                {
                    WARNING << "no target to attack!";
                    setGoal(MicromanagedUnit::GoalType::wait);
                    setStatus(MicromanagedUnit::StatusType::idle);
                    return LATER;
                }
                getUnit()->attack(getTarget());
                setStatus(MicromanagedUnit::StatusType::moving);
                return NEXTTHINK;
            }
            else
            {
                if(getUnit()->isIdle())
                {
                    setStatus(MicromanagedUnit::StatusType::idle);
                }
            }

            return LATER;
        }
        int defaultAttackPosition()
        {
            if(getStatus()!=MicromanagedUnit::StatusType::moving || getTargetPosition().getDistance(getUnit()->getTargetPosition())>32*10)
            {
                if(getTargetPosition()==Positions::None||getTargetPosition()==Positions::Invalid||getTargetPosition()==Positions::Unknown)
                {
                    WARNING << "no position to attack!";
                    setGoal(MicromanagedUnit::GoalType::wait);
                    setStatus(MicromanagedUnit::StatusType::idle);
                    return LATER;
                }
                
                if(getUnit()->isSelected())
                    LOG << "issuing attack position";
                
                getUnit()->attack(getTargetPosition());
                setStatus(MicromanagedUnit::StatusType::moving);
                return NEXTTHINK;
            }
            else
            {
                if(getUnit()->isIdle())
                {
                    setStatus(MicromanagedUnit::StatusType::idle);
                    return LATER;
                }

                if(getUnit()->isUnderAttack())
                {
                    if(getUnit()->getHitPoints() <= getType().maxHitPoints()*(1-getCourage()))
                    {
                        std::set<Unit*> others=getUnit()->getUnitsInRadius(128);
                        Position moveTo(0,0);

                        int count=1;

                        //LOG << getType()<<" is attacked - running away";

                        for(auto it:others)
                        {
                            if(it->getPlayer()==Broodwar->self() || Broodwar->self()->isAlly(it->getPlayer()))
                            {
                                moveTo+=(it->getPosition()-getPosition());
                            }
                            else
                            {
                                moveTo-=(it->getPosition()-getPosition());
                            }
                            count++;
                        }

                        move(getPosition()+Position(moveTo.x()/count,moveTo.y()/count));
                        return NEXTTHINK;
                    }
                }
                else if(getUnit()->isAttacking() || getUnit()->isStartingAttack())
                {
                    if(!getUnit()->isStimmed() && getUnit()->getStimTimer()==0 && getUnit()->getSpellCooldown()==0
                            &&( getType()==UnitTypes::Terran_Marine || getType()==UnitTypes::Terran_Firebat) 
                            && getUnit()->getHitPoints() > getType().maxHitPoints()*(1-getCourage()))
                    {
                        if(getUnit()->canIssueCommand(UnitCommand::useTech(getUnit(),TechTypes::Stim_Packs)))
                        {
                            LOG << getType()<<" is using stim packs with "<<getUnit()->getHitPoints()<<"/"<<getType().maxHitPoints()<<" hitpoints and will wait till frame "<<NEXTTHINK;
                            getUnit()->useTech(TechTypes::Stim_Packs);
                        }
                        return NEXTTHINK;
                    }
                }
                return NOW;
            }

            return LATER;
        }
        int defaultOnThink()
        {
            switch(goal)
            {
                case GoalType::wait:
                    return defaultWait();
                case GoalType::moveToPosition:
                    return defaultMoveToPosition();
                case GoalType::moveToTarget:
                    return defaultMoveToTarget();
                case GoalType::attackPosition:
                    return defaultAttackPosition();
                case GoalType::attackTarget:
                    return defaultAttackTarget();
                default:
                    break;
            }
                
            return LATER;
        }
    };
    
    
    struct MicromanagementHelperAdvancedMovement : public MicromanagementHelper
    {
        int advancedMoveToTarget()
        {
            Broodwar->printf("AdvancedMovement for %s",getUnit()->getType().c_str());
            return LATER;
        }
    };
    
    
    template<const BWAPI::UnitType& T>
    struct MicromanagementInformation :  public MicromanagementHelperDefault, public MicromanagementHelperAdvancedMovement
    {
        MicromanagementInformation(BWAPI::Unit *u)
                : MicromanagedUnit(u,NULL)
        {
            LOG << "Using default micromanagement for "<<u->getType();
        }
        
        int onThink()
        {
            return defaultOnThink();
        }
    };
    template<>
    struct MicromanagementInformation<UnitTypes::Terran_Marine> :  public MicromanagementHelperDefault, public MicromanagementHelperAdvancedMovement
    {
        MicromanagementInformation(BWAPI::Unit *u)
                : MicromanagedUnit(u,NULL)
        {
            LOG << "Using marine micromanagement for "<<u->getType();
        }
        
        int onThink()
        {
            return defaultOnThink();
        }
        
        std::string lastStatusDebugLine;
    };
    
    
    std::set<MicromanagementInformation<UnitTypes::Terran_Marine>*> unitsTerran_Marine;
    std::set<MicromanagementInformation<UnitTypes::AllUnits>*> unitsDefault;

        const char* getStatusText(MicromanagedUnit::StatusType status)
        {
            switch (status)
            {
                case MicromanagedUnit::StatusType::idle:
                    return "i";
                case MicromanagedUnit::StatusType::attacking:
                    return "a";
                case MicromanagedUnit::StatusType::moving:
                    return "m";
                case MicromanagedUnit::StatusType::runningAway:
                    return "r";
                default:
                    return "?";
            }
        }
        const char* getGoalText(MicromanagedUnit::GoalType goal)
        {
            switch (goal)
            {
                case MicromanagedUnit::GoalType::wait:
                    return "w";
                case MicromanagedUnit::GoalType::attackPosition:
                    return "ap";
                case MicromanagedUnit::GoalType::attackTarget:
                    return "at";
                case MicromanagedUnit::GoalType::moveToPosition:
                    return "mp";
                case MicromanagedUnit::GoalType::moveToTarget:
                    return "mt";
                default:
                    return "?";
            }
        }

};
/////////////////////////////////////



MicromanagedUnit *micromanageUnit(BWAPI::Unit* unit)
{
    if(!unit)
    {
        WARNING << "NULL unit to micromange!";
        return NULL;
    }
    
    if(unit->getPlayer() != Broodwar->self())
    {
        WARNING << "tried to micromange unit "<<unit<<" we do not own!";
        return NULL;
    }
    
    if(unit->getClientInfo())
    {
        WARNING << "Unit "<<unit<<" already has some clientInfo attached!";
        return NULL;
    }
    
 
#define ACTIVATE_SPECIAL_TREATMENT_FOR(TYPE)\
    if(unit->getType()==UnitTypes::TYPE)\
    {\
        MicromanagementInformation<UnitTypes::TYPE> *microUnit=new MicromanagementInformation<UnitTypes::TYPE>(unit);\
        units##TYPE.insert(microUnit);\
        return microUnit;\
    }\
    
    ACTIVATE_SPECIAL_TREATMENT_FOR(Terran_Marine)
//    SETUP_MICROMANAGEMENT(Terran_Medic)
//    SETUP_MICROMANAGEMENT(Terran_Firebat)
            
    MicromanagementInformation<UnitTypes::AllUnits> *microUnit=new MicromanagementInformation<UnitTypes::AllUnits>(unit);
    unitsDefault.insert(microUnit);
    return microUnit;
}

MicromanagedUnit::MicromanagedUnit()
    :MicromanagedUnit(NULL,NULL)
{
    
}

MicromanagedUnit::MicromanagedUnit(BWAPI::Unit* u,int (*(*f)[MicromanagedUnit::GoalType::unknown])(MicromanagedUnit*,void*))
    : unit(u),status(StatusType::idle),goal(GoalType::wait),nextThink(NOW),data(),funcs(f),courage(0.2),targetPosition(Positions::None),target(0)
{
    if(!unit)
        WARNING << "MicromanagedUnit created for NULL unit!";
}

MicromanagedUnit::~MicromanagedUnit()
{
/*
    for(int i=0;i<GoalType::unknown;i++)
    {
        if(data[i])
            std::free(data[i]);
    }
*/
}


MicromanagedUnit* MicromanagedUnit::getMicromanagement(BWAPI::Unit* u)
{
    if(!u)
    {
        WARNING << "NULL unit!";
        return NULL;
    }
    
    if(u->getPlayer() != Broodwar->self())
    {
        WARNING << "tried to get micromangement of unit "<<u<<" we do not own!";
        return NULL;
    }
    
    if(!u->getClientInfo())
    {
        WARNING << "Unit "<<u<<" is not micromanaged!";
        return NULL;
    }
    
    return (MicromanagedUnit*)u->getClientInfo();
}


/////////////////////////////////////

void UnitMicromanagementCode::onMatchBegin()
{
}

void UnitMicromanagementCode::onMatchEnd()
{
    for(auto it:unitsTerran_Marine)delete it;unitsTerran_Marine.clear();
    for(auto it:unitsDefault)delete it;unitsDefault.clear();
}

void UnitMicromanagementCode::onTick()
{
    /*
    for(auto it:unitsTerran_Marine)
    {
        if(it->nextThink<=NOW && it->getUnit()->exists())
        {
            if(it->funcs[it->goal])
            {
                it->nextThink=(*it->funcs)[it->goal]((MicromanagedUnit*)it,it->data[it->goal]);
            }
        }
    }
     */
    for(auto it:unitsTerran_Marine)
        if(it->nextThink<=NOW && it->getUnit()->exists())it->onThink();
    for(auto it:unitsDefault)
        if(it->nextThink<=NOW && it->getUnit()->exists())it->onThink();
}

void UnitMicromanagementCode::onDrawPlan()
{
    for(auto it:unitsTerran_Marine)
    {
        if(it->getUnit()->exists())
        {
            BWAPI::Unit *unit=it->getUnit();
            Broodwar->drawBoxMap(unit->getLeft(),unit->getTop(),unit->getRight(),unit->getBottom(),Colors::Teal,false);
            Broodwar->drawTextMap(unit->getLeft(),unit->getTop(),"%s|%s",getGoalText(it->getGoal()),getStatusText(it->getStatus()));
            
            if(unit->isSelected())
            {
                switch(it->getGoal())
                {
                    case MicromanagedUnit::GoalType::attackPosition:
                        Broodwar->drawLineMap(unit->getPosition().x(),unit->getPosition().y(),it->getTargetPosition().x(),it->getTargetPosition().y(),Colors::Red);
                        Broodwar->drawLineMap(unit->getTargetPosition().x(),unit->getTargetPosition().y(),it->getTargetPosition().x(),it->getTargetPosition().y(),Colors::Blue);
                        break;
                    case MicromanagedUnit::GoalType::moveToPosition:
                        Broodwar->drawLineMap(unit->getPosition().x(),unit->getPosition().y(),it->getTargetPosition().x(),it->getTargetPosition().y(),Colors::Green);
                        Broodwar->drawLineMap(unit->getTargetPosition().x(),unit->getTargetPosition().y(),it->getTargetPosition().x(),it->getTargetPosition().y(),Colors::Blue);
                        break;
                    case MicromanagedUnit::GoalType::attackTarget:
                        Broodwar->drawLineMap(unit->getPosition().x(),unit->getPosition().y(),it->getTarget()->getPosition().x(),it->getTarget()->getPosition().y(),Colors::Red);
                        break;
                    case MicromanagedUnit::GoalType::moveToTarget:
                        Broodwar->drawLineMap(unit->getPosition().x(),unit->getPosition().y(),it->getTarget()->getPosition().x(),it->getTarget()->getPosition().y(),Colors::Green);
                        break;
                    default:
                        break;
                }
                std::string debugLine=std::string()
                        +std::string("|")+std::string(unit->isAccelerating()?"ACL":"acl")
                        +std::string("|")+std::string(unit->isBraking()?"BRK":"brk")
                        +std::string("|")+std::string(unit->isMoving()?"MOV":"mov")
                        +std::string("|")+std::string(unit->isStartingAttack()?"STRTATT":"STRTATT")
                        +std::string("|")+std::string(unit->isAttackFrame()?"ATTFRM":"attfrm")
                        +std::string("|")+std::string(unit->isAttacking()?"ATTACK":"attack")
                        +std::string("|")+std::string(unit->isBeingHealed()?"HEALED":"healed")
                        +std::string("|")+std::string(unit->isIdle()?"IDLE":"idle")
                        +std::string("|")+std::string(unit->isInterruptible()?"INTRABLE":"intrable")
                        +std::string("|")+std::string(unit->isStimmed()?"STIM":"stim")
                        +std::string("|")+std::string(unit->isStuck()?"STUCK":"stuck")
                        +std::string("|")+std::string(unit->isUnderAttack()?"UNDRATT":"undratt")
                        +std::string("|");
                if(it->lastStatusDebugLine!=debugLine)
                {
                    LOG << debugLine;
                    it->lastStatusDebugLine=debugLine;
                }
            }
        }
    }
}

void UnitMicromanagementCode::onCheckMemoryLeaks()
{
    
}


