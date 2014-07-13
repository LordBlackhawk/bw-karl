#pragma once

#include "basic-expert.hpp"

class BuildingPlacementExpert : public BasicPortExpert
{
    public:
        void beginTraversal() override;
        void visitRequireSpacePort(RequireSpacePort* port) override;

    protected:
        int                 mapWidth;
        int                 mapHeight;
        BWAPI::UnitType     unitType;
        BWAPI::TilePosition startPos;
        int                 buildDistance;

        BWAPI::TilePosition getBuildLocationNear();
        bool canBuildHereWithSpace(int x, int y);
        bool canBuildHere(int x, int y);
        bool buildable(int x, int y);
        bool movable(int x, int y);
};
