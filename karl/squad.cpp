
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
#include "log.hpp"
#include <BWTA.h>
#include <algorithm>

#include <math.h>
#include <stdlib.h>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

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



bool isPositionWalkable(BWAPI::Position pos)
{
	int x(pos.x()), y(pos.y());
    
	if(!BWAPI::Broodwar->isWalkable(x/8, y/8))
        return false;
	
	std::set<BWAPI::Unit *> units = BWAPI::Broodwar->getUnitsOnTile(x/32, y/32);

    for(auto it:units)
	{
		if(it->getType().isBuilding() 
                || it->getType().isResourceContainer() 
                || !it->getType().isFlyer()) 
		{		
				return false;
		}
	}
	return true;
}



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
        THIS_DEBUG << "Squad::addUnit() called with NULL UnitMicromanagement!!";
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
        THIS_DEBUG << "Squad "<<name<<" got a new member: "<<microUnit->getType();
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
            THIS_DEBUG << "Squad "<<name<<" lost a member: "<<(*it)->getType();
            units.erase(it++);
            continue;
        }
        
        if( ( (*it)->isIdle() &&  (*it)->getTargetPosition().getDistance(defendposition)>8*units.size()/2 )
                || (*it)->getTargetPosition().getDistance(defendposition)>8*units.size())
        {
            bool done=false;
            
            for(int dist=0;dist<8*units.size() && !done;dist++)
            {
                for(int i=-dist;i<=dist;i++)
                {
                    Position p;
                    p=Position(defendposition.x()+i,defendposition.y()-dist);
                    if(isPositionWalkable(p))
                    {
                    	THIS_DEBUG << "Squad "<<name<<" found free spot at "<<i<<" / "<<(-dist);
                        (*it)->attack(p);
                        done=true;
                        break;
                    }
                    p=Position(defendposition.x()+i,defendposition.y()+dist);
                    if(isPositionWalkable(p))
                    {
                    	THIS_DEBUG << "Squad "<<name<<" found free spot at "<<i<<" / "<<(dist);
                        (*it)->attack(p);
                        done=true;
                        break;
                    }
                    p=Position(defendposition.x()-dist,defendposition.y()+i);
                    if(isPositionWalkable(p))
                    {
                    	THIS_DEBUG << "Squad "<<name<<" found free spot at "<<(-dist)<<" / "<<i;
                        (*it)->attack(p);
                        done=true;
                        break;
                    }
                    p=Position(defendposition.x()+dist,defendposition.y()+i);
                    if(isPositionWalkable(p))
                    {
                    	THIS_DEBUG << "Squad "<<name<<" found free spot at "<<dist<<" / "<<(i);
                        (*it)->attack(p);
                        done=true;
                        break;
                    }
                }
            }
            
            if(!done)
            {
            	THIS_DEBUG << "Squad "<<name<<" found no free spot";
            	(*it)->attack(defendposition);
            }
            /*
            BWAPI::Position p=(*it)->getPosition();
            if(p.getDistance(defendposition)>32*4)
                (*it)->attack(defendposition);
             */
        }
        it++;
    }
}




