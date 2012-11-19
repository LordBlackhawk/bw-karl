#pragma once

#include "building-placer.hpp"
#include <BWAPI.h>
#include <BWTA.h>

namespace ArrangementGenerator
{
    struct TilesInRegion
    {
        BWTA::Region* region;
        BWAPI::TilePosition lower, upper;
        TilesInRegion(BWTA::Region* r)
            : region(r)
        {
            lower = BWAPI::TilePosition(mapWidth, mapHeight);
            upper = BWAPI::TilePosition(0,0);
            for (BWAPI::Position it : region->getPolygon()) {
                BWAPI::TilePosition pos(it);
                lower.x() = std::min(lower.x(), pos.x());
                upper.x() = std::max(upper.x(), pos.x());
                lower.y() = std::min(lower.y(), pos.y());
                upper.y() = std::max(upper.y(), pos.y());
            }
        }
        bool init(BWAPI::TilePosition& pos) const
        {
            pos = lower;
            return next(pos);
        }
        bool next(BWAPI::TilePosition& pos) const
        {
            while (nextSimple(pos))
                if (BWTA::getRegion(pos) == region)
                    return true;
            return false;
        }
        bool nextSimple(BWAPI::TilePosition& pos) const
        {
            if (pos.x() < upper.x()) {
                pos.x() += 1;
                return true;
            }
            if (pos.y() < upper.y()) {
                pos.x()  = lower.x();
                pos.y() += 1;
                return true;
            }
            return false;
        }
    };
    
    struct Spiral
    {
        int cx, cy, dx, dy, j, len, maxlen;
        bool first;
        Spiral(const TilePosition& p, int max)
            : cx(p.x()), cy(p.y()), maxlen(max)
        { }
        bool init(BWAPI::TilePosition& pos)
        {
            len = 1;
            j = 0;
            first = true;
            dx = 0;
            dy = 1;
            pos = TilePosition(cx, cy);
            return true;
        }
        bool next(BWAPI::TilePosition& pos)
        {
            while (nextSimple(pos))
                if (pos.x() >= 0 && pos.x() < mapWidth && pos.y() >= 0 && pos.y() < mapHeight)
                    return true;
            return false;
        }
        bool nextSimple(BWAPI::TilePosition& pos)
        {
            pos.x() += dx;
            pos.y() += dy;
            ++j;
            if (j == len) {
                j = 0;
                if (!first)
                    ++len;
                first = !first;
                int dummy = dx;
                dx = dy;
                dy = -dummy;
            }
            return (len < maxlen);
        }
    };
}
