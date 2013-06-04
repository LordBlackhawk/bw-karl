#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

struct Direction
{
    enum Type { O = 0, N = 1, W = 2, S = 3 };
    Type value;
};

struct DualEdge;

struct DualNode
{
    bool checked;
    BWAPI::Position getPosition() const;
    BWTA::Region* getRegion() const;
    bool hasEdge(Direction::Type dir) const;
    DualEdge* getEdge(Direction::Type dir) const;
};

struct DualEdge
{
    int width;
    BWTA::Chokepoint* chokepoint;
    DualNode* getBeginNode() const;
    DualNode* getEndNode() const;
    BWAPI::TilePosition getLeftTile() const;
    BWAPI::TilePosition getRightTile() const;
    BWAPI::Position getPosition() const;
};

DualNode* projectToGraph(const BWAPI::Position& pos);
DualEdge* projectToGraph(const BWAPI::TilePosition& pos, Direction::Type dir);

void addUnitToGraph(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos);
void removeUnitFromGraph(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos);

struct DualGraphCode : public DefaultCode
{
    static void onProgramStart(const char* programname);
    static void onMatchBegin();
    static void onMatchEnd();
    static void onSendText(const std::string& text);
    static void onDrawPlan(HUDTextOutput& hud);
};
