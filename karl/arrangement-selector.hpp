#pragma once

#include <BWAPI.h>
#include <BWTA.h>

namespace ArrangementSelector
{
    struct TakeFirst
    {
        bool operator () (const BWAPI::TilePosition& /*pos*/, bool& cont) const
        {
            cont = false;
            return true;
        }
    };
}
