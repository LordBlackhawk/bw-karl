#pragma once

#include <BWAPI.h>
#include <BWTA.h>

namespace ArrangementSelector
{
    struct TakeFirst
    {
        void init()
        { }
        bool operator () (const BWAPI::TilePosition& /*pos*/, bool& cont) const
        {
            cont = false;
            return true;
        }
    };

    template <class Valuing>
    struct TakeMinimumImpl
    {
        Valuing valuing;
        bool first;
        int bestvalue;
        TakeMinimumImpl(const Valuing& v)
            : valuing(v), first(true), bestvalue(0)
        { }
        void init()
        {
            first = true;
            bestvalue = 0;
        }
        bool operator () (const BWAPI::TilePosition& pos, bool& cont)
        {
            cont = true;
            int value = valuing(pos);
            if (first || (value < bestvalue)) {
                bestvalue = value;
                first     = false;
                return true;
            }
            return false;
        }
    };

    template <class Valuing>
    TakeMinimumImpl<Valuing> TakeMinimum(const Valuing& v)
    {
        return TakeMinimumImpl<Valuing>(v);
    }

    template <class Valuing>
    struct TakeMaximumImpl
    {
        Valuing valuing;
        bool first;
        int bestvalue;
        TakeMaximumImpl(const Valuing& v)
            : valuing(v), first(true), bestvalue(0)
        { }
        void init()
        {
            first = true;
            bestvalue = 0;
        }
        bool operator () (const BWAPI::TilePosition& pos, bool& cont)
        {
            cont = true;
            int value = valuing(pos);
            if (first || (value > bestvalue)) {
                bestvalue = value;
                first     = false;
                return true;
            }
            return false;
        }
    };

    template <class Valuing>
    TakeMaximumImpl<Valuing> TakeMaximum(const Valuing& v)
    {
        return TakeMaximumImpl<Valuing>(v);
    }
}
