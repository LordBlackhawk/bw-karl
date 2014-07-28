#pragma once

#include "utils/array2d.hpp"
#include "utils/time.hpp"
#include <BWAPI.h>
#include <BWTA.h>

class ResourceBoundaryItem;
class RequireSpacePort;

struct BaseLocation
{
    BWTA::BaseLocation*             origin;
    std::set<ResourceBoundaryItem*>  minerals;

    BaseLocation();

    inline BWAPI::TilePosition getTilePosition() const { return origin->getTilePosition(); }
};

struct FieldInformations
{
    bool                buildable   = false;
    bool                creep       = false;
    RequireSpacePort*   blocker     = NULL;
};

struct BlackboardInformations
{
    ~BlackboardInformations();
    void prepare();

    void printFieldInformations(std::ostream& stream);

    Time    lastUpdateTime = -1;
    int     currentMinerals = 0;
    int     currentGas = 0;

    BWAPI::Player* self = NULL;

    std::set<BaseLocation*>     allBaseLocations;
    std::set<BaseLocation*>     ownBaseLocations;

    Array2d<FieldInformations>  fields;
};
