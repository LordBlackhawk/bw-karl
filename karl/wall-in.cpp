// ToDo:
//  * Put code of placement to new arrangement code.

#include "wall-in.hpp"
#include "building-placer.hpp"
#include "log.hpp"
#include "bwta-helper.hpp"
#include "dual-graph.hpp"
#include <cstdlib>
#include <set>
#include <stack>

using namespace BWAPI;
using namespace BWTA;

namespace
{
    struct Graph;

    struct Placement
    {
        TilePosition    pos;
        UnitType        type;

        explicit Placement(const UnitType& t)
            : pos(Positions::Unknown), type(t)
        { }

        bool contains(const TilePosition& p) const
        {
            return    (pos.x() <= p.x()) && (p.x() < pos.x() + type.tileWidth())
                   && (pos.y() <= p.y()) && (p.y() < pos.y() + type.tileHeight());
        }

        bool isBuildable() const
        {
            int mx = pos.x() + type.tileWidth();
            int my = pos.y() + type.tileHeight();
            for (int x=pos.x(); x<mx; ++x)
                for (int y=pos.y(); y<my; ++y)
                    if (!tileInformations[x][y].buildable)
                        return false;
            return true;
        }

        bool intersects(const Placement& other) const
        {
            int mx = pos.x() + type.tileWidth();
            int my = pos.y() + type.tileHeight();
            for (int x=pos.x(); x<mx; ++x)
                for (int y=pos.y(); y<my; ++y)
                    if (other.contains(TilePosition(x, y)))
                        return true;
            return false;
        }

        BWTA::Region* getRegion() const
        {
            return BWTA::getRegion(pos.x(), pos.y());
        }
    };

    struct Graph
    {
        int minX, maxX;
        int minY, maxY;
        int unitWidth, unitHeight;
        std::vector<Placement> placements;

        Graph()
            : minX(0), maxX(0), minY(0), maxY(0)
        { }

        Graph(int lX, int hX, int lY, int hY)
            : minX(lX), maxX(hX), minY(lY), maxY(hY)
        { }
    };

    bool findWay(Graph& graph, BWTA::Chokepoint* chokepoint)
    {
        for (auto it : graph.placements)
            addUnitToGraph(it.type, it.pos);
        bool result = !calcChokepointBlocked(chokepoint, std::max(graph.unitWidth, graph.unitHeight));
        for (auto it : graph.placements)
            removeUnitFromGraph(it.type, it.pos);
        return result;
    }

    int placements = 0;
    bool searchPlacement(BWTA::Chokepoint* chokepoint, BWTA::Region* region, Graph& graph, int deep = 0);

    bool checkPlacementPossible(BWTA::Chokepoint* chokepoint, BWTA::Region* region, Graph& graph, int deep, int x, int y)
    {
        if ((x < 0) || (y < 0))
            return false;

        Placement& p = graph.placements[deep];
        int tileWidth = p.type.tileWidth();
        int tileHeight = p.type.tileHeight();
        if ((x + tileWidth >= mapWidth) || (y + tileHeight >= mapHeight))
            return false;
        
        p.pos.x() = x;
        p.pos.y() = y;

        if (!p.isBuildable())
            return false;

        bool intersection = false;
        for (int k=0; k<deep; ++k)
            if (p.intersects(graph.placements[k]))
        {
            intersection = true;
            break;
        }
        if (intersection)
            return false;

        auto regions = chokepoint->getRegions();
        TilePosition center = TilePosition(regions.first->getCenter());
        if ((x <= center.x()) && (center.x() <= x + tileWidth) && (y <= center.y()) && (center.y() <= y + tileHeight))
            return false;
        center = TilePosition(regions.second->getCenter());
        if ((x <= center.x()) && (center.x() <= x + tileWidth) && (y <= center.y()) && (center.y() <= y + tileHeight))
            return false;

        return searchPlacement(chokepoint, region, graph, deep+1);
    }

    bool searchPlacement(BWTA::Chokepoint* chokepoint, BWTA::Region* region, Graph& graph, int deep)
    {
        if (deep == (int)graph.placements.size()) {
            //LOG << "\tchecking placement...";
            ++placements;
            return !findWay(graph, chokepoint);
        }

        Placement& p = graph.placements[deep];
        if (deep == 0) {
            int xsize = graph.maxX - p.type.tileWidth();
            int ysize = graph.maxY - p.type.tileHeight();
            for (int x=graph.minX; x<xsize; ++x)
                for (int y=graph.minY; y<ysize; ++y)
                    if (checkPlacementPossible(chokepoint, region, graph, deep, x, y)) return true;
        } else {
            for (int k=0; k<deep; ++k) {
                Placement& ps = graph.placements[k];
                int y1 = ps.pos.y() - p.type.tileHeight();
                int y2 = ps.pos.y() + ps.type.tileHeight();
                int x1 = ps.pos.x() - p.type.tileWidth();
                int x2 = ps.pos.x() + ps.type.tileWidth();
                for (int x=x1+1; x<x2; ++x) {
                    if (checkPlacementPossible(chokepoint, region, graph, deep, x, y1)) return true;
                    if (checkPlacementPossible(chokepoint, region, graph, deep, x, y2)) return true;
                }
                for (int y=y1+1; y<y2; ++y) {
                    if (checkPlacementPossible(chokepoint, region, graph, deep, x1, y)) return true;
                    if (checkPlacementPossible(chokepoint, region, graph, deep, x2, y)) return true;
                }
            }
        }

        return false;
    }

    bool searchPlacementIncremental(BWTA::Chokepoint* chokepoint, BWTA::Region* region, Graph& graph, int additional)
    {
        if (searchPlacement(chokepoint, region, graph))
            return true;
        if (additional <= 0)
            return false;

        LOG << "increasing placement size.";

        graph.placements.push_back(Placement(UnitTypes::Terran_Supply_Depot));
        return searchPlacementIncremental(chokepoint, region, graph, additional-1);
    }

    std::set<BWTA::Region*> getRegionsWithinWall(BWTA::Region* region, Chokepoint* cpoint)
    {
        std::set<BWTA::Region*> result;
        std::stack<BWTA::Region*> stack;
        result.insert(region);
        for (auto it : region->getChokepoints())
            if ((it != cpoint) && (getChokepointWidth(it) > 16))
                stack.push(getOtherRegion(it, region));

        while (!stack.empty())
        {
            BWTA::Region* newregion = stack.top();
            stack.pop();
            if (result.count(newregion) == 1)
                continue;

            result.insert(newregion);
            for (auto it : newregion->getChokepoints())
                if (getChokepointWidth(it) > 16)
                    stack.push(getOtherRegion(it, newregion));
        }

        return result;
    }

    bool isSeperating(BWTA::Region* region, Chokepoint* cpoint, BWTA::Region* home)
    {
        std::set<BWTA::Region*> regions = getRegionsWithinWall(region, cpoint);
        for (auto it : getStartLocations()) {
            BWTA::Region* r = it->getRegion();
            if (r != home && regions.count(r) > 0)
                return false;
        }
        return true;
    }

    typedef std::set<std::pair<BWTA::Region*, Chokepoint*>> RegionChokepointPairSet;

    RegionChokepointPairSet getAllSeperatingPairs(BWTA::Region* region, BWTA::Region* home)
    {
        for (auto cp : region->getChokepoints())
            if (isSeperating(region, cp, home))
        {
            RegionChokepointPairSet result = getAllSeperatingPairs(getOtherRegion(cp, region), home);
            result.insert(std::make_pair(region, cp));
            return result;
        }
        return RegionChokepointPairSet();
    }

    std::vector<Graph> allGraphs;
}

std::set<BuildingPositionPrecondition*> designWallIn(BWTA::Region* region, Chokepoint* cpoint)
{
    TilePosition pos(cpoint->getCenter());
    Graph graph(std::max(0, pos.x() - 7), std::min(pos.x() + 7, mapWidth),
                std::max(0, pos.y() - 7), std::min(pos.y() + 7, mapHeight));

    graph.unitWidth  = 18;
    graph.unitHeight = 18;
    allGraphs.push_back(graph);

    if (!findWay(graph, cpoint)) {
        WARNING << "No way found without buildings!";
        return std::set<BuildingPositionPrecondition*>();
    }

    graph.placements.push_back(Placement(UnitTypes::Terran_Barracks));
    graph.placements.push_back(Placement(UnitTypes::Terran_Supply_Depot));
    if (!searchPlacementIncremental(cpoint, region, graph, 1))
        return std::set<BuildingPositionPrecondition*>();

    std::set<BuildingPositionPrecondition*> result;
    for (auto it : graph.placements)
        result.insert(getBuildingPosition(it.type, it.pos));
    return result;
}

std::set<BuildingPositionPrecondition*> designWallIn()
{
    BWTA::Region* home = getStartLocation(Broodwar->self())->getRegion();
    RegionChokepointPairSet pairs = getAllSeperatingPairs(home, home);

    LOG << "Found " << pairs.size() << " possible chokepoints for the wall-in.";

    std::set<BuildingPositionPrecondition*> result;
    for (auto it : pairs) {
        result = designWallIn(it.first, it.second);
        if (!result.empty())
            break;
    }
    LOG << "checked " << placements << " placements.";
    return result;
}

void WallInCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    for (auto it : allGraphs) {
        int x1 = it.minX*32;
        int y1 = it.minY*32;
        int x2 = it.maxX*32;
        int y2 = it.maxY*32;
        Broodwar->drawBoxMap(x1, y1, x2, y2, Colors::Green, false);
    }
}

