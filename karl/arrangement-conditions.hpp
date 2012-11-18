#pragma once

#include "building-placer.hpp"
#include <BWAPI.h>
#include <functional>

namespace ArrangementTileCondition
{
    #define DEF(name)                                                                                           \
        struct name : public std::binary_function<const BWAPI::TilePosition&, const TileInformation&, bool>     \
        {   bool operator () (const TilePosition& position, const TileInformation& info) const; };              \
        bool name::operator () (const TilePosition& position, const TileInformation& info) const

    DEF(IsReserved)
    {
        return info.reserved;
    }

    #undef DEF
}

namespace ArrangementCondition
{
    template <class TC>
    struct CheckTileConditionImpl : public std::binary_function<const BWAPI::TilePosition&, const BWAPI::UnitType&, bool>
    {
        TC tc;
        CheckTileConditionImpl(const TC& c)
            : tc(c)
        { }
        bool operator () (const BWAPI::TilePosition& position, const BWAPI::UnitType& type) const
        {
            int maxX = position.x() + type.tileWidth();
            if (maxX >= mapWidth)
                return false;
            int maxY = position.y() + type.tileHeight();
            if (maxY >= mapHeight)
                return false;

            for(int x=position.x(); x<maxX; ++x)
                for(int y=position.y(); y<maxY; ++y)
            {
                BWAPI::TilePosition pos(x, y);
                TileInformation& info = tileInformations[pos];
                if (!tc(pos, info))
                    return false;
            }
        }
    };

    template <class TC>
    CheckTileConditionImpl<TC> CheckTileCondition(const TC& c)
    {
        return CheckTileConditionImpl<TC>(c);
    }

    #define DEF(name)                                                                                           \
        struct name : public std::binary_function<const BWAPI::TilePosition&, const TileInformation&, bool>     \
        {   bool operator () (const TilePosition& position, const TileInformation& info) const; };              \
        bool name::operator () (const TilePosition& position, const TileInformation& info) const

    DEF(CheckPower)
    {
        if (type.requiresPsi())
            return true;
        return getPowerTime(position, type) >= Precondition::Max;
    }

    #undef DEF
}
