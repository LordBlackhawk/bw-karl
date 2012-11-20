#pragma once

#include "object-counter.hpp"
#include <BWAPI.h>
#include <BWTA.h>

class Arrangement : public ObjectCounter<Arrangement>
{
    protected:
        int refcounter;

    public:
        Arrangement(int rc)
            : refcounter(rc)
        { }

        void decRef()
        {
            --refcounter;
            if (refcounter == 0)
                delete this;
        }

        virtual ~Arrangement()
        { }

        virtual void reset() = 0;
        virtual BWAPI::TilePosition onTick(int id) = 0;
};
