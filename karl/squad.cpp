
// ToDo:
// *

#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "squad.hpp"
#include "unit-micromanagement.hpp"
#include "unit-trainer.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "object-counter.hpp"
#include "mineral-line.hpp"
#include <BWTA.h>
#include <algorithm>
#include <cassert>

#include <math.h>
#include <stdlib.h>

using namespace BWAPI;

namespace
{
    std::set<Squad*> squads;

}

Squad* getSquadByName(const std::string& name)
{
    for (auto it : squads)
        if (it->getName() == name)
            return it;
    return NULL;
}

/////////////////////////////////////


void SquadCode::onMatchBegin()
{
}

void SquadCode::onMatchEnd()
{
    squads.clear();
}

void SquadCode::onTick()
{
    for(auto sq=squads.begin();sq!=squads.end();sq++)
    {
        (*sq)->onTick();
    }
}

void SquadCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    for(auto sq=squads.begin();sq!=squads.end();sq++)
    {
        (*sq)->onDrawPlan();
    }
}

void SquadCode::onCheckMemoryLeaks()
{
}




//////////////////////////////////////

Squad::Squad()
{
        //keep track of all squads around
    squads.insert(this);
}

Squad::~Squad()
{
    /*for(auto it:units)
    {
            //notify all units that this squad no longer exists...
        //it->_assignToSquad(NULL);   
    }*/
    
        //remove us from the squad list
    squads.erase(this);
}

BWAPI::Position Squad::getCenter()
{
    
    if(units.size()>0)
    {
        Position ret(0,0);
        
        for(auto it=units.begin();it!=units.end();it++)
        {
            ret+=(*it)->getPosition();
        }
        
        return Position(ret.x()/units.size(),ret.y()/units.size());
    }
    else
        return defendposition;
}

int Squad::getUnitCount(const BWAPI::UnitType& ut)
{
    int ret=0;
    
    for(auto it=units.begin();it!=units.end();it++)
    {
        if((*it)->getType()==ut)
            ret++;
    }
    
    return ret;
}


void Squad::defend(BWAPI::Position pos)
{
    defendposition=pos;
}

/*
void Squad::addUnit(UnitMicromanagement* u)
{
    if(u)
    {
        units.insert(u);
        u->_assignToSquad(this);
    }
    else
        LOG << "Squad::addUnit() called with NULL UnitMicromanagement!!";
}
*/

void Squad::addUnit(BWAPI::Unit* u)
{
    if(u)
    {
        addUnit(micromanageUnit(u));
        //UnitMicromanagement *um=(UnitMicromanagement*)u->getClientInfo();
        //if(um)
        //{
        //    addUnit(um);
        //}
        //else
        //    WARNING << "Squad::addUnit() called with unit without UnitMicromanagement!!";
    }
    else
        WARNING << "Squad::addUnit() called with NULL unit!!";
}

void Squad::addUnit(MicromanagedUnit *microUnit)
{
    if(microUnit)
    {
        LOG << "Squad "<<name<<" got a new member: "<<microUnit->getType();
        units.insert(microUnit);
        //microUnit->_assignToSquad(this);
    }
    else
        WARNING << "Squad::addUnit() called with NULL MicromanagedUnit!!";
}

std::string Squad::getName() const
{
    return name;
}

void Squad::setName(std::string desc)
{
    name=desc;
}


void Squad::onDrawPlan()
{
    BWAPI::Position center=getCenter();
    
    for(auto it=units.begin();it!=units.end();it++)
    {
        BWAPI::Position p=(*it)->getPosition();
        
        Broodwar->drawLineMap(center.x(),center.y(),p.x(),p.y(),Colors::Grey);
    }
    
    Broodwar->drawLineMap(center.x(),center.y(),defendposition.x(),defendposition.y(),Colors::Yellow);
    Broodwar->drawTextMap(center.x(),center.y(),"[%s]",name.c_str());
}

void Squad::onTick()
{
    for(auto it=units.begin();it!=units.end();)
    {
        if(!(*it)->exists())
        {
            LOG << "Squad "<<name<<" lost a member: "<<(*it)->getType();
            units.erase(it++);
            continue;
        }
        
        if((*it)->isIdle() || (*it)->getTargetPosition().getDistance(defendposition)>32*3)
        {
            BWAPI::Position p=(*it)->getPosition();
            if(p.getDistance(defendposition)>32*4)
                (*it)->attack(defendposition);
        }
        it++;
    }
}




