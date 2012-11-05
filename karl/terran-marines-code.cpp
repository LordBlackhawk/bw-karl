
// ToDo:
// *

#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "terran-marines-code.hpp"
#include "unit-trainer.hpp"
#include "squad.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "object-counter.hpp"
#include "mineral-line.hpp"
#include "unit-micromanagement.hpp"
#include <BWTA.h>
#include <algorithm>
#include <cassert>

#include <math.h>
#include <stdlib.h>



using namespace BWAPI;

namespace
{
    UnitMicromanagement *simpleScout=NULL;
    int scoutAttacked=0;
    
    Squad *scoutProtection=NULL,*baseProtection=NULL;
    
    std::set<UnitPrecondition*> infantry;

}


void doSomethingUsefulWithInfantry(UnitPrecondition* u)
{
    infantry.insert(u);
}


void TerranMarinesCode::onMatchBegin()
{
}

void TerranMarinesCode::onMatchEnd()
{
    simpleScout=0;
    
    release(scoutProtection);
    release(baseProtection);
}

void TerranMarinesCode::onTick()
{
    if(!baseProtection)
    {
        LOG << "creating base protection";
        baseProtection = new Squad();
        baseProtection->setName("base");

        BWTA::BaseLocation* base=BWTA::getStartLocation(Broodwar->self());
        BWTA::BaseLocation* otherbase=NULL;
        
        for(auto sl : BWTA::getStartLocations())
        {
            if(sl!=base)
            {
                otherbase=sl;
                break;
            }
        }
        
        if(!otherbase)
            LOG << "No other base found!";
        
        BWTA::Region* region=base->getRegion();

            //only one choke from starting position? follow this path until we reach bigger ground
        if(region->getChokepoints().size()>1)
            Broodwar->printf("FIXME: more than one choke from starting point - only protecting the nearest one.");
        
        auto ckit=region->getChokepoints().begin();
        BWTA::Chokepoint *choke=*ckit;
        
        
        ckit++; //find choke closest to otherbase
        while(ckit!=region->getChokepoints().end())
        {
            double currentDistance=BWTA::getGroundDistance((TilePosition)choke->getCenter(),otherbase->getTilePosition());
            double newDistance=BWTA::getGroundDistance((TilePosition)(*ckit)->getCenter(),otherbase->getTilePosition());
            
            if(newDistance>0 && newDistance<currentDistance)
            {
                choke=*ckit;
            }
            ckit++;
        }
       
        LOG << "following choke path";

        if(choke->getRegions().first==region)
            region=choke->getRegions().second;
        else 
            region=choke->getRegions().first;

        while(region->getChokepoints().size()==2)
        {
            auto it=region->getChokepoints().begin();
            if(*it == choke)    //old choke? -> take the other!
            {
                it++;
            }
            choke=*it;

                //step on to next region
            if(choke->getRegions().first==region)
                region=choke->getRegions().second;
            else 
                region=choke->getRegions().first;
        }

        baseProtection->defend(choke->getCenter());
    }
    
    if(!scoutProtection)
    {
        LOG << "creating ScoutProtection";
        scoutProtection = new Squad();
        scoutProtection->setName("scout");
        scoutProtection->defend(baseProtection->getCenter());
    }
    
    if(!simpleScout)
    {
        for(auto it:infantry)
        {
            if(it->ut==UnitTypes::Terran_Marine && it->isFulfilled() && it->unit && it->unit->getClientInfo() && it->unit->exists())
            {
                Broodwar->printf("got a simple scout.");
                simpleScout=(UnitMicromanagement*)it->unit->getClientInfo();
                infantry.erase(it);
                break;
            }
        }
        //simpleScout=getIdleUnit(UnitTypes::Terran_Marine);
    }
    
    if(simpleScout)
    {
        BWAPI::Unit *m=simpleScout->getUnit();
        
        if(!simpleScout->exists())
        {
            Broodwar->printf("simple scout lost.");
            simpleScout=0;
            return;
        }
        
        if(m->isUnderAttack())
        {
            scoutAttacked=Broodwar->getFrameCount();
            
                //run back to our protectors
            if(!m->isMoving() || m->getTargetPosition().getDistance(scoutProtection->getCenter()) > 10)
            {
                m->move(scoutProtection->getCenter());
                
                if(scoutProtection->getUnitCount(UnitTypes::Terran_Marine)>12)
                {
                    scoutProtection->defend(m->getPosition());
                    Broodwar->printf("simple scout calling for help.");
                }
                else
                    Broodwar->printf("simple scout running away.");
            }
        }
        else if(scoutAttacked && !m->isIdle())
        {
            if(Broodwar->getFrameCount()-scoutAttacked>100)
            {
                Broodwar->printf("seems simple scout got away.");
                scoutAttacked=0;
                m->move(scoutProtection->getCenter());
            }
        }
        else if(m->isIdle())
        {
            std::set<BWTA::BaseLocation*> bls=BWTA::getStartLocations();
            int take=((unsigned)rand())%bls.size();
            
            for ( auto it=bls.begin() ; it != bls.end() ; it++ )
            {
                if((take--)==0)
                {
                    BWTA::BaseLocation* c=*it;
                    
                    if(c==BWTA::getStartLocation(Broodwar->self()))
                        continue;   //do not scout our own base
                    
                    Position p=c->getPosition();
                    
                    if(Broodwar->isVisible(p.x()/TILE_SIZE,p.y()/TILE_SIZE))
                        continue;   //do not scout what we currently see already
                    
                    if(p.hasPath(m->getPosition()))
                    {
                        Broodwar->printf("simple scout going for next target.");
                        m->attack(p);
                        break;
                    }
                }
            }
        }
    }


    for(auto it:infantry)
    {
        if(it->isFulfilled() && it->unit)
        {
            if(it->unit->getClientInfo())
            {
                if(baseProtection->getUnitCount(it->ut)<5 || baseProtection->getUnitCount(it->ut)<scoutProtection->getUnitCount(it->ut))
                {
                    //Broodwar->printf("adding %s (%i) to base protection", it->ut.getName().c_str(),it->unit->getID());
                    baseProtection->addUnit(it->unit);
                }
                else
                {
                    //Broodwar->printf("adding %s (%i) to scout protection", it->ut.getName().c_str(),it->unit->getID());
                    scoutProtection->addUnit(it->unit);
                }
                infantry.erase(it);
                release(it);
                break;
            }
            else
                Broodwar->printf("%s ready but no clientinfo yet", it->ut.getName().c_str());
        }
    }
    
    //Broodwar->printf("waiting for %i infantry units",infantry.size());
    
}

void TerranMarinesCode::onDrawPlan()
{
    if(simpleScout)
    {
        Unit *m=simpleScout->getUnit();
        Position pos=m->getPosition(),dest=m->getTargetPosition();
        
        Broodwar->drawLineMap(pos.x(),pos.y(),dest.x(),dest.y(),Colors::Yellow);
    }
}

void TerranMarinesCode::onCheckMemoryLeaks()
{
}
