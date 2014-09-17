#pragma once

#include "utils/array2d.hpp"
#include "utils/time.hpp"
#include <BWAPI.h>
#include <BWTA.h>

class BlackboardInformations;
class ResourceBoundaryItem;
class RequireSpacePort;

struct BaseLocation
{
    public:
        BWTA::BaseLocation*             origin;
        std::set<ResourceBoundaryItem*> minerals;

        BaseLocation(BlackboardInformations* o);
        Time lastSeenComplete() const;
        bool isCompleteExplored() const;

        inline BWAPI::TilePosition getTilePosition() const { return origin->getTilePosition(); }
        inline BWAPI::Position getPosition() const { return BWAPI::Position(getTilePosition()); }

    protected:
        BlackboardInformations* owner;
};

struct FieldInformations
{
    Time                lastSeen    = -1;
    bool                buildable   = false;
    bool                creep       = false;
    bool                movable     = false;
    bool                subtiles[4][4];
    RequireSpacePort*   blocker     = NULL;

    inline bool isExplored() const { return lastSeen > 1; }
    inline bool isMovable() const { return movable && (blocker == NULL); }
};

struct BlackboardInformations
{
    ~BlackboardInformations();
    void prepare();
    void fieldSeen(const BWAPI::TilePosition& tp, bool creep);

    void printFieldInformations(std::ostream& stream);

    Time    lastUpdateTime = -1;
    int     currentMinerals = 0;
    int     currentGas = 0;

    int unusedLarvaCount = 0;   //number of available ProvideUnitPorts providing larva
    int workerCount = 0;    //number of available ProvideUnitPorts providing workers

    BWAPI::Player* self = NULL;
    BWAPI::Player* neutral = NULL;

    std::set<BaseLocation*>     allBaseLocations;
    std::set<BaseLocation*>     ownBaseLocations;

    Array2d<FieldInformations>  fields;
};
