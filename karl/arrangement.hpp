#pragma once

#include <BWAPI.h>
#include <BWTA.h>

class Arrangement
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

        virtual BWAPI::TilePosition onTick(int id) = 0;
};
