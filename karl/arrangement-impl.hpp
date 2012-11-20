#pragma once

#include "arrangement.hpp"
#include "system-helper.hpp"
#include <typeinfo>

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
            : Arrangement(1), gen(g), cond(c), sel(s), status(init), pos(BWAPI::TilePositions::Unknown), best(BWAPI::TilePositions::Unknown)
        { }

        virtual ~ArrangementImpl()
        { }

        virtual void reset()
        {
            status = init;
            pos = best = BWAPI::TilePositions::Unknown;
        }

        bool condCall(const BWAPI::TilePosition& pos)
        {
            if (pos.x() < 0 || pos.x() >= mapWidth || pos.y() < 0 || pos.y() >= mapHeight) {
                LOG << "Generator '" << demangle(typeid(Generator).name()) << "' returned tile out of range: " << pos;
                return false;
            }

            TileInformation& info = tileInformations[pos];
            return cond(pos, info);
        }

        bool isBestValid()
        {
            if (best == BWAPI::TilePositions::Unknown)
                return false;
            return condCall(best);
        }

        virtual BWAPI::TilePosition onTick(int /*id*/)
        {
            if ((status == finished) && !isBestValid())
                reset();

            if (status == init) {
                sel.init();
                status = gen.init(pos) ? searching : finished;
            }

            if (status == finished)
                return best;

            const int max = 1000;
            int counter = 0;
            while (++counter < max) {
                if (condCall(pos)) {
                    bool cont = true;
                    if (sel(pos, cont))
                        best = pos;
                    if (!cont) {
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
