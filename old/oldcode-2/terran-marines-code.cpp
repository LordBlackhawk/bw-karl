
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
#include "string-helper.hpp"
#include "container-helper.hpp"
#include "log.hpp"
#include <BWTA.h>
#include <algorithm>

#include <math.h>
#include <stdlib.h>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

namespace
{
    //UnitMicromanagement *simpleScout=NULL;
    //int scoutAttacked=0;
    
    Squad *scoutProtection=NULL,*baseProtection=NULL;
    
    std::set<UnitPrecondition*> infantry;

}


void doSomethingUsefulWithInfantry(UnitPrecondition* u)
{
    if (u != NULL)
        infantry.insert(u);
    else
        WARNING << "Called doSomethingUsefulWithInfantry with NULL.";
}


void TerranMarinesCode::onMatchBegin()
{
}

void TerranMarinesCode::onMatchEnd()
{
    //simpleScout=0;
    Containers::clear_and_delete(infantry);
    
    release(scoutProtection);
    release(baseProtection);
}

void TerranMarinesCode::onTick()
{
    if(!baseProtection)
    {
        THIS_DEBUG << "creating base protection";
        baseProtection = new Squad();
        baseProtection->setName("base");

        BWTA::BaseLocation* base=BWTA::getStartLocation(Broodwar->self());
        BWTA::BaseLocation* otherbase=NULL;
        
        if(!base)
            THIS_DEBUG << "No base found!";
        else
        {
            for(auto sl : BWTA::getStartLocations())
            {
                if(sl!=base)
                {
                    otherbase=sl;
                    break;
                }
            }
        }

        if(!otherbase)
        {
            THIS_DEBUG << "No other base found!";
        }
        
        if (base && otherbase) 
        {
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
           
            THIS_DEBUG << "following choke path";

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

            Squad::Line defendLine=Squad::Line(choke->getSides().first,choke->getSides().second);
            BWAPI::Position sideward(defendLine.getSideward().x()/4,defendLine.getSideward().y()/4);
            
                // orientation correct?
            if(region->getCenter().getDistance(defendLine.getCenter()+defendLine.getForward()) < region->getCenter().getDistance(defendLine.getCenter()-defendLine.getForward()))
            {
                baseProtection->defend(Squad::Line(choke->getSides().first+sideward,choke->getSides().second-sideward));
            }
            else
            {
                baseProtection->defend(Squad::Line(choke->getSides().second-sideward,choke->getSides().first+sideward));
            }
        } 
        else 
        {
            baseProtection->defend(Position(16*Broodwar->mapWidth(), 16*Broodwar->mapHeight()));
        }
    }
    
    if(!scoutProtection)
    {
        THIS_DEBUG << "creating ScoutProtection";
        scoutProtection = new Squad();
        scoutProtection->setName("scout");
        scoutProtection->defend(baseProtection->getDefendLine());
    }
    
/*
    if(!simpleScout)
    {
        for(auto it:infantry)
        {
            if(it->ut==UnitTypes::Terran_Marine && it->isFulfilled() && it->unit && it->unit->getClientInfo() && it->unit->exists())
            {
                Broodwar->printf("got a simple scout.");
                simpleScout=(UnitMicromanagement*)it->unit->getClientInfo();
                infantry.erase(it);
                release(it);
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
*/

    for(auto it:infantry)
    {
        if(it->isFulfilled() && it->unit)
        {
            //if(it->unit->getClientInfo())
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
            //else
            //    Broodwar->printf("%s ready but no clientinfo yet", it->ut.getName().c_str());
        }
    }
    
    static bool K_MULTIPLY_pressed=false;
    static BWAPI::Position protectScoutPos;
    if(Broodwar->getKeyState(Key::K_MULTIPLY))
    {
        static int timeo=0;
        if(!K_MULTIPLY_pressed)
        {
            protectScoutPos=Broodwar->getMousePosition()+Broodwar->getScreenPosition();
            K_MULTIPLY_pressed=true;
        }
        
        if(scoutProtection && ((timeo++)%100==0))
            scoutProtection->defend(protectScoutPos,Broodwar->getMousePosition()+Broodwar->getScreenPosition());
    }
    else
    {
        if(K_MULTIPLY_pressed)
        {
            if(scoutProtection)
                scoutProtection->defend(protectScoutPos,Broodwar->getMousePosition()+Broodwar->getScreenPosition());
        }
        K_MULTIPLY_pressed=false;
    }
            
    
    static bool K_DIVIDE_pressed=false;
    static BWAPI::Position protectBasePos;
    if(Broodwar->getKeyState(Key::K_DIVIDE))
    {
        static int timeo=0;
        if(!K_DIVIDE_pressed)
        {
            protectBasePos=Broodwar->getMousePosition()+Broodwar->getScreenPosition();
            K_DIVIDE_pressed=true;
        }
        if(baseProtection && ((timeo++)%100==0))
            baseProtection->defend(protectBasePos,Broodwar->getMousePosition()+Broodwar->getScreenPosition());
    }
    else
    {
        if(K_DIVIDE_pressed)
        {
            if(baseProtection)
                baseProtection->defend(protectBasePos,Broodwar->getMousePosition()+Broodwar->getScreenPosition());
        }
        K_DIVIDE_pressed=false;
    }
    
    if(Broodwar->getKeyState(Key::K_SUBTRACT))
    {
        Broodwar->setLocalSpeed(-1);
    }
    if(Broodwar->getKeyState(Key::K_ADD))
    {
        Broodwar->setLocalSpeed(5);
    }
    //Broodwar->printf("waiting for %i infantry units",infantry.size());
    
}

void TerranMarinesCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
/*
    if(simpleScout)
    {
        Unit *m=simpleScout->getUnit();
        Position pos=m->getPosition(),dest=m->getTargetPosition();
        
        Broodwar->drawLineMap(pos.x(),pos.y(),dest.x(),dest.y(),Colors::Yellow);
    }
*/
}


void TerranMarinesCode::onCheckMemoryLeaks()
{
}

void TerranMarinesCode::onSendText(const std::string& text)
{
    std::vector<std::string> words = splitIntoWords(text);
    if (words[0] != "/attack")
        return;

    if (words.size() < 2) {
        Broodwar->printf("The attack command needs the squad to command as argument.");
        return;
    }

    Squad* squad = getSquadByName(words[1]);
    if (squad == NULL) {
        Broodwar->printf("Squad '%s' not found!", words[1].c_str());
        return;
    }
    
    Position pos = Broodwar->getScreenPosition();
    pos.x() += 320;
    pos.y() += 200;
    squad->defend(pos);
}
