#pragma once

#include "building-placer.hpp"
#include <BWAPI.h>
#include <functional>

namespace ArrangementCondition
{
    typedef std::binary_function<const BWAPI::TilePosition&, const TileInformation&, bool> ConditionFunctionType;

    #define DEF(name)                                                                                           \
        struct name : public ConditionFunctionType                                                              \
        {   bool operator () (const BWAPI::TilePosition& position, const TileInformation& info) const; };       \
        bool name::operator () (const BWAPI::TilePosition& position, const TileInformation& info) const

    DEF(IsBuildable)
    {
        (void) position;
        return info.buildable;
    }
    
    DEF(IsBuilding)
    {
        (void) position;
        return (info.building != NULL);
    }
        
    DEF(IsReserved)
    {
        (void) position;
        return (info.pre != NULL);
    }
    
    DEF(TileHasPower)
    {
        (void) position;
        return (info.energytime < Precondition::Max);
    }
    
    DEF(TileHasCreep)
    {
        (void) position;
        return (info.creeptime < Precondition::Max);
    }
    
    DEF(NoResourcesNear)
    {
        (void) info;
        int left   = position.x();
        int top    = position.y();

        for (auto m : Broodwar->getStaticMinerals())
        {
            if (Broodwar->isVisible(m->getInitialTilePosition()) ||
                Broodwar->isVisible(m->getInitialTilePosition().x() + 1, m->getInitialTilePosition().y()))
            {
                if (!m->isVisible())
                    continue; // tile position is visible, but mineral is not => mineral does not exist
            }
            if (m->getInitialTilePosition().x() > left - 5 &&
                m->getInitialTilePosition().y() > top  - 4 &&
                m->getInitialTilePosition().x() < left + 7 &&
                m->getInitialTilePosition().y() < top  + 6)
            {
                return false;
            }
        }
        for (auto g : Broodwar->getStaticGeysers())
        {
            if (g->getInitialTilePosition().x() > left - 7 &&
                g->getInitialTilePosition().y() > top  - 5 &&
                g->getInitialTilePosition().x() < left + 7 &&
                g->getInitialTilePosition().y() < top  + 6)
            {
                return false;
            }
        }
        return true;
    }

    #undef DEF
    
    struct InRegion : public ConditionFunctionType
    {
        BWTA::Region* region;
        InRegion(BWTA::Region* r)
            : region(r)
        { }
        bool operator () (const BWAPI::TilePosition& position, const TileInformation& info) const
        {
            (void) info;
            return (BWTA::getRegion(position) == region);
        }
    };
    
    struct IsReachable : public ConditionFunctionType
    {
        BWTA::Region* region;
        IsReachable(BWTA::Region* r)
            : region(r)
        { }
        bool operator () (const BWAPI::TilePosition& position, const TileInformation& info) const
        {
            (void) info;
            return region->isReachable(BWTA::getRegion(position));
        }
    };

    template <class TC>
    struct CheckNeighbourhoodImpl : public ConditionFunctionType
    {
        TC tc;
        int lx, rx, ly, ry;
        CheckNeighbourhoodImpl(const TC& c, int lx_, int rx_, int ly_, int ry_)
            : tc(c), lx(lx_), rx(rx_), ly(ly_), ry(ry_)
        { }
        bool operator () (const BWAPI::TilePosition& position, const TileInformation& info) const
        {
            (void) info;

            int minX = position.x() + lx;
            if (minX < 0)
                return false;
            int maxX = position.x() + rx;
            if (maxX >= mapWidth)
                return false;
            int minY = position.y() + ly;
            if (minY < 0)
                return false;
            int maxY = position.y() + ry;
            if (maxY >= mapHeight)
                return false;

            for(int x=minX; x<maxX; ++x)
                for(int y=minY; y<maxY; ++y)
            {
                BWAPI::TilePosition pos(x, y);
                TileInformation& subinfo = tileInformations[pos];
                if (!tc(pos, subinfo))
                    return false;
            }
            return true;
        }
    };
    
    template <class TC>
    CheckNeighbourhoodImpl<TC> CheckNeighbourhood(int lx, int rx, int ly, int ry, const TC& c)
    {
        return CheckNeighbourhoodImpl<TC>(c, lx, rx, ly, ry);
    }

    template <class TC>
    CheckNeighbourhoodImpl<TC> CheckUnitTiles(const BWAPI::UnitType& type, const TC& c)
    {
        return CheckNeighbourhood(0, type.tileWidth(), 0, type.tileHeight(), c);
    }
    
    template <class TC>
    CheckNeighbourhoodImpl<TC> CheckUnitTilesWithAddon(const BWAPI::UnitType& type, const TC& c)
    {
        int width = type.tileWidth() + (type.canBuildAddon() ? 2 : 0);
        return CheckNeighbourhood(0, width, 0, type.tileHeight(), c);
    }

    template <class TC>
    CheckNeighbourhoodImpl<TC> CheckUnitTilesWithSpace(const BWAPI::UnitType& type, int s, const TC& c)
    {
        return CheckNeighbourhood(-s, type.tileWidth()+s, -s, type.tileHeight()+s, c);
    }
    
    template <class TC>
    CheckNeighbourhoodImpl<TC> CheckUnitTilesWithAddonWithSpace(const BWAPI::UnitType& type, int s, const TC& c)
    {
        int width = type.tileWidth() + (type.canBuildAddon() ? 2 : 0);
        return CheckNeighbourhood(-s, width+s, -s, type.tileHeight()+s, c);
    }
    
    template <class TC>
    CheckNeighbourhoodImpl<TC> CheckRelativePos(int dx, int dy, const TC& c)
    {
        return CheckNeighbourhood(dx, dx+1, dy, dy+1, c);
    }
    
    CheckNeighbourhoodImpl<TileHasPower> HasPower(const BWAPI::UnitType& type)
    {
        return CheckRelativePos(type.tileWidth()/2, type.tileHeight()/2, TileHasPower());
    }
}
