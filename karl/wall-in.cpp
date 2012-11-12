#include "wall-in.hpp"
#include "building-placer.hpp"
#include "log.hpp"
#include "bwta-helper.hpp"
#include <cstdlib>
#include <set>
#include <stack>

using namespace BWAPI;
using namespace BWTA;

namespace
{
    struct Graph;
    struct Node;
    struct Edge;

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

        int space(int d) const
        {
            if (type == UnitTypes::Terran_Supply_Depot) {
                int values[] = { 9, 10, 10, 5 };
                return values[d];
            }

            if (type == UnitTypes::Terran_Barracks) {
                int values[] = { 7, 8, 16, 15 };
                return values[d];
            }

            return 0;
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

        int space(const TilePosition& pos, int d) const
        {
            if (pos.x() < 0 || pos.y() < 0)
                return 0;

            TileInformation& info = tileInformations[pos.x()][pos.y()];
            if (!info.buildable) {
                return info.space(d);
            } else {
                for (auto it : placements)
                    if (it.contains(pos))
                        return it.space(d);
                return 32;
            }
        }
    };

    struct Edge
    {
        int x, y;
        bool dirx;

        Edge(int x_, int y_, bool dx)
            : x(x_), y(y_), dirx(dx)
        { }

        TilePosition getTilePosition(bool left) const
        {
            if (left)
                return TilePosition(x - (dirx ? 1 : 0), y);
            else
                return TilePosition(x, y - (dirx ? 0 : 1));
        }

        int space(const Graph& graph) const
        {
            return   graph.space(getTilePosition(true),  dirx ? 3 : 2)
                   + graph.space(getTilePosition(false), dirx ? 1 : 0);
        }

        bool isWalkable(const Graph& graph) const
        {
            int unitSize = dirx ? graph.unitWidth : graph.unitHeight;
            return (unitSize < space(graph));
        }
    };

    struct Node
    {
        int x, y;
        
        Node()
            : x(0), y(0)
        { }

        Node(int x_, int y_)
            : x(x_), y(y_)
        { }

        Edge getEdges(int n) const
        {
            switch (n)
            {
                case 0: return Edge(x,   y,   true);
                case 1: return Edge(x,   y-1, false);
                case 2: return Edge(x-1, y,   true);
                case 3: return Edge(x,   y,   false);
                default:
                    WARNING << "Called getEdges with wrong n";
                    exit(99);
            }
        }

        Node getNodeOfEdge(int n) const
        {
            switch (n)
            {
                case 0: return Node(x+1, y);
                case 1: return Node(x,   y-1);
                case 2: return Node(x-1, y);
                case 3: return Node(x,   y+1);
                default:
                    WARNING << "Called getNodeOfEdge with wrong n";
                    exit(99);
            }
        }

        bool isInsideOf(const Placement& p) const
        {
            return    (p.pos.x() < x) && (x < p.pos.x() + p.type.tileWidth())
                   && (p.pos.y() < y) && (y < p.pos.y() + p.type.tileHeight());
        }

        bool isWithin(const Graph& graph) const
        {
            if (!(     (graph.minX <= x) && (x <= graph.maxX)
                    && (graph.minY <= y) && (y <= graph.maxY)))
                return false;

            for (auto it : graph.placements)
                if (isInsideOf(it))
                    return false;

            return true;
        }

        bool inRegion(BWTA::Region* region) const
        {
            for (int k=x-1; k<=x; ++k)
                for (int l=y-1; l<=y; ++l)
                    if (getRegion(TilePosition(k, l)) != region)
                        return false;
            return true;
        }

        bool operator == (const Node& other) const
        {
            return (x == other.x) && (y == other.y);
        }

        bool operator < (const Node& other) const
        {
            if (x < other.x)
                return true;
            if (x > other.x)
                return false;
            return (y < other.y);
        }
    };

    bool isWalkable(const Graph& graph, const Node& node)
    {
        return node.isWithin(graph);
    }

    bool isWalkable(const Graph& graph, const Edge& edge)
    {
        return edge.isWalkable(graph);
    }
    
    std::set<Node> visited_debug;

    bool findWay(const Graph& graph, const Node& start, const Node& end)
    {
        std::set<Node> visited;
        std::stack<Node> stack;
        stack.push(start);
        while (!stack.empty())
        {
            Node next = stack.top();
            stack.pop();
            if (visited.count(next) == 1)
                continue;

            visited.insert(next);
            for (int i=0; i<4; ++i) {
                Edge edge = next.getEdges(i);
                Node node = next.getNodeOfEdge(i);
                if (isWalkable(graph, edge) && isWalkable(graph, node)) {
                    if (node == end)
                        return true;
                    stack.push(node);
                }
            }
        }
        visited_debug = visited;
        LOG << "No way found!!!";
        return false;
    }
    
    int placements = 0;

    bool searchPlacement(BWTA::Region* region, Graph& graph, const Node& start, const Node& end, int deep = 0)
    {
        if (deep == (int)graph.placements.size()) {
            //LOG << "\tchecking placement...";
            ++placements;
            return !findWay(graph, start, end);
        }

        Placement& p = graph.placements[deep];
        for (int x=graph.minX+2; x<graph.maxX-p.type.tileWidth()-2; ++x)
            for (int y=graph.minY+2; y<graph.maxY-p.type.tileHeight()-2; ++y)
        {
            p.pos.x() = x;
            p.pos.y() = y;
            if (p.getRegion() != region)
                continue;

            if (!p.isBuildable())
                continue;

            bool intersection = false;
            for (int k=0; k<deep; ++k)
                if (p.intersects(graph.placements[k]))
            {
                intersection = true;
                break;
            }
            if (intersection)
                continue;

            if (searchPlacement(region, graph, start, end, deep+1))
                return true;
        }

        return false;
    }

    bool searchPlacementIncremental(BWTA::Region* region, Graph& graph, const Node& start, const Node& end, int additional)
    {
        if (searchPlacement(region, graph, start, end))
            return true;
        if (additional <= 0)
            return false;
        
        LOG << "increasing placement size.";

        graph.placements.push_back(Placement(UnitTypes::Terran_Supply_Depot));
        return searchPlacementIncremental(region, graph, start, end, additional-1);
    }

    Node findNodeInRegion(const Graph& graph, BWTA::Region* region)
    {
        for (int x=graph.minX+1; x<graph.maxX-1; ++x) {
            Node node(x, graph.minY+1);
            if (node.inRegion(region))
                return node;

            node = Node(x, graph.maxY-1);
            if (node.inRegion(region))
                return node;
        }

        for (int y=graph.minY+1; y<graph.maxY-1; ++y) {
            Node node(graph.minX+1, y);
            if (node.inRegion(region))
                return node;

            node = Node(graph.maxX-1, y);
            if (node.inRegion(region))
                return node;
        }

        assert(false);
    }

    std::set<BWTA::Region*> getRegionsWithinWall(BWTA::Region* region, Chokepoint* cpoint)
    {
        std::set<BWTA::Region*> result;
        std::stack<BWTA::Region*> stack;
        result.insert(region);
        for (auto it : region->getChokepoints())
            if (it != cpoint)
                stack.push(getOtherRegion(it, region));

        while (!stack.empty())
        {
            BWTA::Region* newregion = stack.top();
            stack.pop();
            if (result.count(newregion) == 1)
                continue;

            result.insert(newregion);
            for (auto it : newregion->getChokepoints())
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

    Graph lastGraph;
    Node  lastStart;
    Node  lastEnd;
}

std::set<BuildingPositionPrecondition*> designWallIn(BWTA::Region* region, Chokepoint* cpoint)
{
    TilePosition pos(cpoint->getCenter());
    BWTA::Region* other = getOtherRegion(cpoint, region);
    Graph graph(std::max(0, pos.x() - 9), std::min(pos.x() + 9, mapWidth),
                std::max(0, pos.y() - 9), std::min(pos.y() + 9, mapHeight));
    Node start = findNodeInRegion(graph, region);
    Node end   = findNodeInRegion(graph, other);

    graph.unitWidth  = 23;
    graph.unitHeight = 19;

    lastGraph = graph;
    lastStart = start;
    lastEnd   = end;
    
    if (!findWay(graph, start, end)) {
        WARNING << "No way found without buildings!";
        return std::set<BuildingPositionPrecondition*>();
    }

    graph.placements.push_back(Placement(UnitTypes::Terran_Barracks));
    graph.placements.push_back(Placement(UnitTypes::Terran_Supply_Depot));
    if (!searchPlacementIncremental(region, graph, start, end, 1))
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
    /*
    int x1 = lastGraph.minX*32;
    int y1 = lastGraph.minY*32;
    int x2 = lastGraph.maxX*32;
    int y2 = lastGraph.maxY*32;
    Broodwar->drawBoxMap(x1, y1, x2, y2, Colors::Green, false);

    int sx = lastStart.x*32;
    int sy = lastStart.y*32;
    int ex = lastEnd.x*32;
    int ey = lastEnd.y*32;
    Broodwar->drawCircleMap(sx, sy, 5, Colors::Green, true);
    Broodwar->drawCircleMap(ex, ey, 5, Colors::Red,   true);

    for (auto it : visited_debug) {
        int x = it.x*32;
        int y = it.y*32;
        Broodwar->drawCircleMap(x, y, 3, Colors::Yellow, true);
    }
    */
}
