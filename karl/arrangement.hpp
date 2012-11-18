#pragma once

#include <BWAPI.h>
#include <BWTA.h>

class Arrangement
{
    public:
        virtual ~Arrangement() = 0;
        
        virtual void onTick() = 0;
};
