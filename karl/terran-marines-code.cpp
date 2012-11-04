
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
#include "utils/debug.h"
#include "mineral-line.hpp"
#include <BWTA.h>
#include <algorithm>
#include <cassert>

#include <math.h>
#include <stdlib.h>



using namespace BWAPI;

namespace
{
    UnitPrecondition *simpleScout=NULL;
    int scoutAttacked=0;
    
    Squad *scoutProtection=NULL,*baseProtection=NULL;
}


void TerranMarinesCode::onMatchBegin()
{
}

void TerranMarinesCode::onMatchEnd()
{
    release(simpleScout);
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
    
    if(!simpleScout && nextUnitAvaiable(UnitTypes::Terran_Marine)==0 )
    {
        Broodwar->printf("got a simple scout.");
        simpleScout=getIdleUnit(UnitTypes::Terran_Marine);
    }
    
    if(simpleScout)
    {
        Unit *m=simpleScout->unit;
        
        if(!m || !m->exists() || !simpleScout->isFulfilled())
        {
            Broodwar->printf("simple scout lost.");
            release(simpleScout);
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

#define ADD_UNIT(TYPE) \
    if(nextUnitAvaiable(TYPE)==0)\
    {\
        UnitPrecondition *pre=getIdleUnit(TYPE);\
        \
        if(baseProtection->getUnitCount(TYPE)<5 || baseProtection->getUnitCount(TYPE)<scoutProtection->getUnitCount(TYPE))\
        {\
            Broodwar->printf("adding %s to base protection", TYPE.getName().c_str()); \
            baseProtection->addUnit(pre->unit);\
        }\
        else\
        {\
            Broodwar->printf("adding %s to scout protection", TYPE.getName().c_str()); \
            scoutProtection->addUnit(pre->unit);\
        }\
        release(pre);\
    }\
    
    ADD_UNIT(UnitTypes::Terran_Marine);
    ADD_UNIT(UnitTypes::Terran_Firebat);
    ADD_UNIT(UnitTypes::Terran_Medic);
}

void TerranMarinesCode::onDrawPlan()
{
    if(simpleScout && simpleScout->isFulfilled() && simpleScout->unit)
    {
        Unit *m=simpleScout->unit;
        Position pos=m->getPosition(),dest=m->getTargetPosition();
        
        Broodwar->drawLineMap(pos.x(),pos.y(),dest.x(),dest.y(),Colors::Yellow);
    }
}

void TerranMarinesCode::onCheckMemoryLeaks()
{
}
