// ToDo:
//  *

#include "idle-unit-container.hpp"
#include "code-list.hpp"
#include "mineral-line.hpp"
#include "larvas.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "addon-builder.hpp"
#include "container-helper.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include <algorithm>
#include <vector>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

namespace
{
    std::set<Unit*>                 idleunits;

    UnitPrecondition* createAndErase(Unit* unit)
    {
        idleunits.erase(unit);
        return new UnitPrecondition(unit);
    }

    std::vector<UnitPrecondition*>    waitingfor;

    UnitPrecondition* eraseFromWaiting(UnitPrecondition* r)
    {
        Containers::remove(waitingfor, r);
        return r;
    }

    bool updateWaiting(UnitPrecondition* pre)
    {
        if (pre->isFulfilled()) {
            idleunits.insert(pre->unit);
            
            if(!pre->unit)
                WARNING << "Unit is NULL in precondition";
            
            THIS_DEBUG << "Unit " <<(pre->unit?pre->unit->getID():0) <<": " << pre->unit->getType() << " added to idle units.";

            release(pre);
            return true;
        }
        return false;
    }
    
    bool searchWorker(Unit* w)
    {
        if (!w->getType().isWorker())
            return false;
        
        useWorker(w);
        return true;
    }
    
    bool fulfillsModifier(Unit* u, const UnitPrecondition::ModifierType& mod)
    {
        if (mod == UnitPrecondition::WhatEver)
            return true;
        
        bool hasAddon   = (u->getAddon() != NULL);
        bool needsAddon = (mod == UnitPrecondition::WithAddon);
        return (hasAddon == needsAddon);
    }
    
    bool fulfillsModifier(UnitPrecondition* p, const UnitPrecondition::ModifierType& mod)
    {
        if (mod == UnitPrecondition::WhatEver)
            return true;
        
        bool hasAddon   = (p->mod == UnitPrecondition::WithAddon);
        bool needsAddon = (mod    == UnitPrecondition::WithAddon);
        return (hasAddon == needsAddon);
    }
    
    const char* modToStr(const UnitPrecondition::ModifierType& mod)
    {
        switch (mod)
        {
            case UnitPrecondition::WithAddon:
                return "with addon";
            case UnitPrecondition::WithoutAddon:
                return "without addon";
            case UnitPrecondition::WhatEver:
                return "what ever";
            default:
                return "unknown value";
        }
    }
}

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut, const UnitPrecondition::ModifierType& mod)
{
    if (ut == UnitTypes::Zerg_Larva)
        return getLarva();

    for (auto it : idleunits)
        if ((it->getType() == ut) && (fulfillsModifier(it, mod)))
            return createAndErase(it);

    for (auto it : waitingfor)
        if ((it->ut == ut) && (fulfillsModifier(it, mod)))
            return eraseFromWaiting(it);

    WARNING << "No idle unit of type " << ut << " (" << modToStr(mod) << ") found.";
    return NULL;
}

int nextUnitAvailable(const BWAPI::UnitType& ut)
{
    for (auto it : idleunits)
        if (it->getType() == ut)
            return 0;

    int result = Precondition::Impossible;
    for (auto it : waitingfor)
        if (it->ut == ut)
            result = std::min(result, it->time);

    return result;
}

void rememberIdle(Unit* unit)
{
    if (unit->getPlayer() != Broodwar->self())
        return;
    if (!unit->exists())
        return;

    if (unit->getType().isWorker())
        useWorker(unit);
    idleunits.insert(unit);
    THIS_DEBUG << "Unit " << unit->getID() << ": " << unit->getType() << " added to idle units.";
}

void rememberIdle(UnitPrecondition* unit)
{
    if (unit == NULL)
        return;

    waitingfor.push_back(unit);
}

UnitPrecondition* rememberFirst(const std::pair<UnitPrecondition*, UnitPrecondition*>& unit)
{
    if (unit.first == NULL)
        WARNING << "called rememberFirst with first == NULL.";
    rememberIdle(unit.first);
    return unit.second;
}

UnitPrecondition* rememberSecond(const std::pair<UnitPrecondition*, UnitPrecondition*>& unit)
{
    if (unit.second == NULL)
        WARNING << "called rememberSecond with second == NULL.";
    rememberIdle(unit.second);
    return unit.first;
}

UnitPrecondition* createUnit(const BWAPI::UnitType& ut)
{
    if (ut.isRefinery()) {
        buildRefinery(ut);
        return NULL;
    } else if (ut.whatBuilds().first.isWorker()) {
        auto result = buildUnit(ut);
        if (result.second != NULL)
            useWorker(result.second);
        return result.first;
    } else if (ut.getRace() == Races::Zerg) {
        return morphUnit(ut);
    } else if (ut.isAddon()) {
        return rememberSecond(buildAddon(ut));
    } else {
        return rememberSecond(trainUnit(ut));
    }
}

void IdleUnitContainerCode::onMatchEnd()
{
    idleunits.clear();
    Containers::clear_and_delete(waitingfor);
}

void IdleUnitContainerCode::onTick()
{
    Containers::remove_if(waitingfor, updateWaiting);
    
    if (Broodwar->getFrameCount() % 30 != 0)
        return;
    
    Containers::remove_if(idleunits, searchWorker);
}

void IdleUnitContainerCode::onUnitCreate(BWAPI::Unit* unit)
{
    if (unit->getPlayer() != Broodwar->self())
        return;

    if (CodeList::onAssignUnit(unit))
        return;

    if (unit->getType().isWorker() && (Broodwar->getFrameCount() == 0))
        return;

    idleunits.insert(unit);
    THIS_DEBUG << "Unit " <<unit->getID() <<": " << unit->getType() << " added to idle units.";
}

void IdleUnitContainerCode::onUnitDestroy(BWAPI::Unit* unit)
{
    Player* self = Broodwar->self();
    if (unit->getPlayer() != self)
        return;

    idleunits.erase(unit);
}
