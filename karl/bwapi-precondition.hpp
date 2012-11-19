#pragma once

#include "precondition.hpp"
#include <BWAPI.h>

struct ResourcesPrecondition : public Precondition
{
    int             minerals;
    int             gas;
    std::string     debugname;
    
    ResourcesPrecondition(int t, int m, int g)
        : Precondition(t), minerals(m), gas(g)
    { }
};

struct SupplyPrecondition : public Precondition
{
    BWAPI::Race     race;
    int             supply;
    std::string     debugname;
    
    SupplyPrecondition(const BWAPI::Race& r, int s)
        : race(r), supply(s)
    { }
};

struct BuildingPositionPrecondition : public Precondition
{
    BWAPI::UnitType     ut;
    BWAPI::TilePosition pos;

    BuildingPositionPrecondition(int it, const BWAPI::UnitType& t, const BWAPI::TilePosition& p)
        : Precondition(it), ut(t), pos(p)
    { }
    
    bool isExplored() const
    {
        int x = pos.x(), y = pos.y();
        for (int dx=0; dx<ut.tileWidth(); ++dx)
            for (int dy=0; dy<ut.tileHeight(); ++dy)
                if (!BWAPI::Broodwar->isExplored(x+dx, y+dy))
                    return false;
        return true;
    }
};

struct RequirementsPrecondition : public Precondition
{
    BWAPI::UnitType     ut;
    BWAPI::UpgradeType  gt;
    int                 level;
    
    RequirementsPrecondition(const BWAPI::UnitType& t)
        : Precondition(0), ut(t), gt(BWAPI::UpgradeTypes::None), level(0)
    { }
    
    RequirementsPrecondition(const BWAPI::UpgradeType& t, int l)
        : Precondition(0), ut(BWAPI::UnitTypes::None), gt(t), level(l)
    { }
};

struct UnitPrecondition : public Precondition
{
    enum ModifierType { WithAddon, WithoutAddon, WhatEver };

    BWAPI::UnitType     ut;
    BWAPI::Position     pos;
    BWAPI::Position     wishpos;
    BWAPI::Unit*        unit;
    ModifierType        mod;
    std::string         debugname;

    UnitPrecondition()
        : Precondition(0), pos(BWAPI::Positions::Unknown), wishpos(BWAPI::Positions::Unknown), unit(NULL), mod(WhatEver)
    { }

    explicit UnitPrecondition(BWAPI::Unit* u)
        : Precondition(0), ut(u->getType()), pos(u->getTilePosition()), wishpos(BWAPI::Positions::Unknown), unit(u), 
          mod((u->getAddon() != NULL) ? WithAddon : WithoutAddon)
    { }
    
    UnitPrecondition(const BWAPI::UnitType& t, const BWAPI::Position& p, BWAPI::Unit* u, const ModifierType& mt, const std::string& dn = "")
        : Precondition(0), ut(t), pos(p), wishpos(BWAPI::Positions::Unknown), unit(u), mod(mt), debugname(dn)
    { }
    
    UnitPrecondition(int time, const BWAPI::UnitType& t, const BWAPI::Position& p, const ModifierType& mt, const std::string& dn = "")
        : Precondition(time), ut(t), pos(p), wishpos(BWAPI::Positions::Unknown), unit(NULL), mod(mt), debugname(dn)
    { }
};
