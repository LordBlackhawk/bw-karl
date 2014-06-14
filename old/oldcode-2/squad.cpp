
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

#define THIS_DEBUG LOG

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
        return defendLine.getCenter();
}

Squad::Line Squad::getDefendLine()
{
    return defendLine;
}

int Squad::getUnitCount()
{
    return units.size();
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

void Squad::defend(BWAPI::Position p1,BWAPI::Position p2)
{
    defend(Line(p1,p2));
}

void Squad::defend(BWAPI::Position pos)
{
    defend(Line(pos,pos));
}

void Squad::defend(Line d)
{
    defendLine=d;
    haveToRecalculatePositions=true;
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
        haveToRecalculatePositions=true;
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
    
    Broodwar->drawLineMap(defendLine.p[0].x(),defendLine.p[0].y(),defendLine.p[1].x(),defendLine.p[1].y(),Colors::Yellow);
    BWAPI::Position direction=defendLine.getCenter()+defendLine.getForward();
    Broodwar->drawLineMap(defendLine.getCenter().x(),defendLine.getCenter().y(),direction.x(),direction.y(),Colors::Yellow);
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
            haveToRecalculatePositions=true;
            continue;
        }

        it++;
    }
   
    if(!(units.size()>0))
        haveToRecalculatePositions=false;
   
    if(haveToRecalculatePositions)
    {
        std::set<MicromanagedUnit*> closeCombat;
        std::set<MicromanagedUnit*> rangeCombat;
        std::set<MicromanagedUnit*> airCombat;
        
        for(auto it=units.begin();it!=units.end();it++)
        {
            if((*it)->getType().isFlyer())
            {
                airCombat.insert(*it);
            }
            else if((*it)->getType().groundWeapon().maxRange()>32)
            {
                rangeCombat.insert(*it);
            }
            else
            {
                closeCombat.insert(*it);
            }
        }

#define UNITSIZE 48
        
      	THIS_DEBUG << "Squad "<<name<<": recalculation positions for "<<closeCombat.size()<<" close, "<<rangeCombat.size()<<" ranged and "<<airCombat.size()<<" air combat units.";
        
        int spacing=1;
        double defendLength=defendLine.getLength();
        BWAPI::Position sideward=defendLine.getSideward();
        BWAPI::Position forward=defendLine.getForward();
        
        if(sideward.getLength()<1)sideward=BWAPI::Position(8,0);
        if(forward.getLength()<1)forward=BWAPI::Position(0,8);

        int tx=-1,ty=0;
        int wx=(int)(defendLength/UNITSIZE)+1;
        if(wx<2)wx=2;
        
#define MAPTTOPOS(x) (defendLine.p[0].x()+UNITSIZE*tx*sideward.x()/8-UNITSIZE*ty*forward.x()/8)
#define _NEXTPOS tx+=spacing; if(tx>=wx+ty){ty++;tx=-ty;}
            //FIXME: this may hang if we can not find any suitable movement postion!
#define NEXTPOS do{_NEXTPOS}while(!Broodwar->isWalkable(MAPTTOPOS(x)/8,MAPTTOPOS(y)/8));      
        
        tx=-1; ty=0;
        if(closeCombat.size()>1)
        {
            spacing=(int)(defendLength/((closeCombat.size()-1)*UNITSIZE));
            if( spacing < 1 )spacing=1;
            NEXTPOS
        }
        else
            tx=wx/2;
        
        for(auto it=closeCombat.begin();it!=closeCombat.end();it++)
        {
            (*it)->attack(BWAPI::Position(MAPTTOPOS(x)-(*it)->getType().dimensionLeft(),MAPTTOPOS(y)-(*it)->getType().dimensionUp()));
            THIS_DEBUG << tx << " / "<< ty<<" of "<<wx<<" at "<<(*it)->getTargetPosition().x()<<","<<(*it)->getTargetPosition().y();
            NEXTPOS
        }
        
        tx=-1; ty++;
        if(rangeCombat.size()>1)
        {
            spacing=(int)(defendLength/(rangeCombat.size()*UNITSIZE));
            if( spacing < 1 )spacing=1;
            NEXTPOS
        }
        else
            tx=wx/2;
        for(auto it=rangeCombat.begin();it!=rangeCombat.end();it++)
        {
            (*it)->attack(BWAPI::Position(MAPTTOPOS(x)-(*it)->getType().dimensionLeft(),MAPTTOPOS(y)-(*it)->getType().dimensionUp()));
            THIS_DEBUG << tx << " / "<< ty<<" of "<<wx<<" at "<<(*it)->getTargetPosition().x()<<","<<(*it)->getTargetPosition().y();
            NEXTPOS
        }
        
        for(auto it=airCombat.begin();it!=airCombat.end();it++)
        {
            (*it)->attack(defendLine.getCenter());
        }
        
#undef MAPTTOPOS
#undef _NEXTPOS
#undef NEXTPOS
#undef UNITSIZE
        
        haveToRecalculatePositions=false;
    }
}




