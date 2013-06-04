#include "dual-graph.hpp"
#include "building-placer.hpp"
#include "log.hpp"
#include <cassert>

using namespace BWAPI;
using namespace BWTA;

namespace
{
    DualNode*   baseNode;
    DualEdge*   baseEdge;
    int         xsize;
    int         ysize;

    void readDualGraph()
    {
        int size = xsize * (ysize - 1) + (xsize - 1) * ysize;
        for (int k=0; k<size; ++k)
            baseEdge[k].width = 0;

        int mapWidth = xsize - 1, mapHeight = ysize - 1;
        for (int x=0; x<mapWidth; ++x)
            for (int y=0; y<mapHeight; ++y)
        {
            TilePosition pos(x, y);
            TileInformation& info = tileInformations[x][y];
            projectToGraph(pos, Direction::O)->width += info.space(0);
            projectToGraph(pos, Direction::N)->width += info.space(1);
            projectToGraph(pos, Direction::W)->width += info.space(2);
            projectToGraph(pos, Direction::S)->width += info.space(3);
        }

        for (int k=0; k<size; ++k) {
            DualEdge* edge = &(baseEdge[k]);
            edge->chokepoint = (edge->getBeginNode()->getRegion() == edge->getEndNode()->getRegion())
                                    ? getNearestChokepoint(edge->getPosition()) : NULL;
        }
    }
}

void addUnitToGraph(const UnitType& ut, const TilePosition& pos)
{
    int sx = pos.x(), ex = sx + ut.tileWidth()-1;
    int sy = pos.y(), ey = sy + ut.tileHeight()-1;

    for (int x=sx; x<=ex; ++x) {
        projectToGraph(TilePosition(x, sy), Direction::N)->width += 16 * ut.tileHeight() - 32 - ut.dimensionUp();
        projectToGraph(TilePosition(x, ey), Direction::S)->width += 16 * ut.tileHeight() - 32 - ut.dimensionDown();
    }

    for (int y=sy; y<=ey; ++y) {
        projectToGraph(TilePosition(sx, y), Direction::W)->width += 16 * ut.tileWidth() - 32 - ut.dimensionLeft();
        projectToGraph(TilePosition(ex, y), Direction::O)->width += 16 * ut.tileWidth() - 32 - ut.dimensionRight();
    }

    for (int x=sx; x<=ex; ++x)
        for (int y=sy; y<=ey; ++y)
    {
        if (x != ex) projectToGraph(TilePosition(x, y), Direction::O)->width = 0;
        if (y != ey) projectToGraph(TilePosition(x, y), Direction::S)->width = 0;
    }
}

void removeUnitFromGraph(const UnitType& ut, const TilePosition& pos)
{
    int sx = pos.x(), ex = sx + ut.tileWidth()-1;
    int sy = pos.y(), ey = sy + ut.tileHeight()-1;

    for (int x=sx; x<=ex; ++x) {
        projectToGraph(TilePosition(x, sy), Direction::N)->width -= 16 * ut.tileHeight() - 32 - ut.dimensionUp();
        projectToGraph(TilePosition(x, ey), Direction::S)->width -= 16 * ut.tileHeight() - 32 - ut.dimensionDown();
    }

    for (int y=sy; y<=ey; ++y) {
        projectToGraph(TilePosition(sx, y), Direction::W)->width -= 16 * ut.tileWidth() - 32 - ut.dimensionLeft();
        projectToGraph(TilePosition(ex, y), Direction::O)->width -= 16 * ut.tileWidth() - 32 - ut.dimensionRight();
    }

    for (int x=sx; x<=ex; ++x)
        for (int y=sy; y<=ey; ++y)
    {
        if (x != ex) projectToGraph(TilePosition(x, y), Direction::O)->width = 64;
        if (y != ey) projectToGraph(TilePosition(x, y), Direction::S)->width = 64;
    }
}

DualNode* nodeByIndex(int x, int y);
bool indexByNode(const DualNode* node, int& x, int& y);
DualEdge* edgeByIndex(bool hor, int x, int y);
bool indexByEdge(const DualEdge* edge, bool& hor, int& x, int& y);

#include "dual-graph-node.hpp"
#include "dual-graph-edge.hpp"
#include "dual-graph-search.hpp"
#include "dual-graph-test.hpp"

void DualGraphCode::onProgramStart(const char* /*programname*/)
{
    testDualGraph();
}

void DualGraphCode::onMatchBegin()
{
    xsize = Broodwar->mapWidth() + 1;
    ysize = Broodwar->mapHeight() + 1;
    baseNode = new DualNode[xsize * ysize];
    baseEdge = new DualEdge[xsize * (ysize - 1) + (xsize - 1) * ysize];
    readDualGraph();
}

void DualGraphCode::onMatchEnd()
{
    delete baseNode;
    baseNode = NULL;
    delete baseEdge;
    baseEdge = NULL;
}

namespace
{
    bool showGraph = true;
}

void DualGraphCode::onSendText(const std::string& text)
{
    if (text == "/show graph")
        showGraph = !showGraph;
}

void DualGraphCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    if (!showGraph) return;

    Position topleft = Broodwar->getScreenPosition();
    int left   = topleft.x()/32;
    int right  = std::min(xsize-1, left + 640/32);
    int top    = topleft.y()/32;
    int bottom = std::min(ysize-1, top + 480/32);
    for (int x=left; x<=right; ++x)
        for (int y=top; y<=bottom; ++y)
    {
        TilePosition pos(x, y);
        DualEdge* edge = projectToGraph(pos, Direction::N);
        if ((edge->width > 15 && edge->width != 64) || (edge->chokepoint != NULL)) {
            Position dp = edge->getBeginNode()->getPosition();
            Broodwar->drawLineMap(dp.x(), dp.y(), dp.x() + 32, dp.y(), (edge->chokepoint != NULL) ? Colors::Red : Colors::Grey);
            Broodwar->drawTextMap(dp.x() + 14, dp.y() - 4, "%d", edge->width);
        }

        edge = projectToGraph(pos, Direction::W);
        if ((edge->width > 15 && edge->width != 64) || (edge->chokepoint != NULL)) {
            Position dp = edge->getBeginNode()->getPosition();
            Broodwar->drawLineMap(dp.x(), dp.y(), dp.x(), dp.y() + 32, (edge->chokepoint != NULL) ? Colors::Red : Colors::Grey);
            Broodwar->drawTextMap(dp.x() - 4, dp.y() + 14, "%d", edge->width);
        }
    }
}
