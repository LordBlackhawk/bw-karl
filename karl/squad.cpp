
// ToDo:
// *

#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "squad.hpp"
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

void SquadCode::onDrawPlan()
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
    squads.insert(this);
}

Squad::~Squad()
{
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


void Squad::addUnit(BWAPI::Unit* u)
{

    if(u)
        units.insert(u);
    else
        LOG << "Squad::addUnit() called with NULL unit!!";
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
            units.erase(it++);
            continue;
        }
        
        if((*it)->isIdle())
        {
            BWAPI::Position p=(*it)->getPosition();
            if(p.getDistance(defendposition)>10)
                (*it)->attack(defendposition);
        }
        it++;
    }
}




