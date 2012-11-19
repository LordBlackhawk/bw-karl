#pragma once

#include "arrangement.hpp"

template <class Generator, class Condition, class Selector>
class ArrangementImpl : public Arrangement
{
    friend class BuildingPositionInternal;

    protected:
        enum StatusType { init, searching, finished };
        
        Generator                       gen;
        Condition                       cond;
        Selector                        sel;
        StatusType                      status;
        BWAPI::TilePosition             pos;
        BWAPI::TilePosition             best;

    public:
        ArrangementImpl(const Generator& g, const Condition& c, const Selector& s)
            : Arrangement(1), gen(g), cond(c), sel(s), status(init), pos(BWAPI::Positions::Unknown), best(BWAPI::Positions::Unknown)
        { }
        
        virtual ~ArrangementImpl()
        { }
        
        virtual BWAPI::TilePosition onTick(int /*id*/)
        {
            if (status == init)
                status = gen.init(pos) ? searching : finished;
            
            if (status == finished)
                return best;
            
            const int max = 1000;
            int counter = 0;
            while (++counter < max) {
                TileInformation& info = tileInformations[pos];
                if (cond(pos, info)) {
                    bool c = true;
                    if (sel(pos, c))
                        best = pos;
                    if (!c) {
                        status = finished;
                        break;
                    }
                }
                if (!gen.next(pos)) {
                    status = finished;
                    break;
                }
            }
            
            return best;
        }
};

template <class Generator, class Condition, class Selector>
Arrangement* createArrangement(const Generator& g, const Condition& c, const Selector& s)
{
    return new ArrangementImpl<Generator, Condition, Selector>(g, c, s);
}
