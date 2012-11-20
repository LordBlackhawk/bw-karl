#pragma once

#include "valuing.hpp"
#include <BWAPI.h>
#include <BWTA.h>
#include <functional>

namespace ArrangementValuing
{
    typedef std::unary_function<const BWAPI::TilePosition&, int>    ValuingFunctionType;
    
    struct DistanceTo : public ValuingFunctionType
    {
        BWAPI::TilePosition base;
        DistanceTo(const BWAPI::TilePosition& b)
            : base(b)
        { }
        int operator () (const BWAPI::TilePosition& pos) const
        {
            return (int) base.getDistance(pos);
        }
    };

    struct InRegionV : public ValuingFunctionType
    {
        BWTA::Region* region;
        InRegionV(BWTA::Region* r)
            : region(r)
        { }
        int operator () (const BWAPI::TilePosition& pos) const
        {
            return (BWTA::getRegion(pos) == region) ? 1 : 0;
        }
    };

    struct CallValueExpo : public ValuingFunctionType
    {
        BWTA::BaseLocation* home;
        CallValueExpo(BWTA::BaseLocation* h)
            : home(h)
        { }
        int operator () (const BWAPI::TilePosition& pos) const
        {
            BWTA::BaseLocation* base = BWTA::getNearestBaseLocation(pos);   
            return valueExpo(base, home);
        }
    };
}
