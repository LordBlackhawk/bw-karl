#pragma once

#include "object-counter.hpp"
#include <BWAPI.h>
#include <BWTA.h>

namespace
{
    struct BuildingPositionInternal;
}

class Arrangement : public ObjectCounter<Arrangement>
{
    public:
        virtual ~Arrangement()
        { }

        virtual void releasePrecondition(int id) = 0;
        virtual void registerPrecondition(int id, BuildingPositionInternal* pre) = 0;
        virtual void reset() = 0;
        virtual void onTick(int id) = 0;
};
