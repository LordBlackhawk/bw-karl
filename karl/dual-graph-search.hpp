#include <algorithm>

void resetChecked()
{
    int size = xsize * ysize;
    for (int k=0; k<size; ++k)
        baseNode[k].checked = false;
}

namespace
{
    std::map<BWTA::Chokepoint*, std::pair<int, int>>    chokepointWidths;
}

int getChokepointWidth(BWTA::Chokepoint* chokepoint)
{
    auto it = chokepointWidths.find(chokepoint);
    assert( it != chokepointWidths.end() );
    return std::min(it->second.first, it->second.second);
}

void initChokepointValues()
{
    for (auto it : BWTA::getChokepoints())
        chokepointWidths[it] = std::make_pair(0,0);
}

void setChokepointValue(BWTA::Chokepoint* chokepoint, BWTA::Region* region, int width, bool override)
{
    auto it = chokepointWidths.find(chokepoint);
    assert( it != chokepointWidths.end() );
    if (chokepoint->getRegions().first == region) {
        if (override || (it->second.first < width))
            it->second.first = width;
    } else {
        if (override || (it->second.second < width))
            it->second.second = width;
    }
}

void recursiveSetChokepointValue(DualNode* node, int width)
{
    if (node->checked) return;
    node->checked = true;

    for (int k=0; k<4; ++k)
        if (node->hasEdge((Direction::Type)k))
    {
        DualEdge* edge = node->getEdge((Direction::Type)k);
        if (edge->chokepoint != NULL) {
            setChokepointValue(edge->chokepoint, node->getRegion(), width, false);
        } else if (edge->width >= width) {
            recursiveSetChokepointValue(edge->getOtherNode(node), width);
        }
    }
}

void recalcRegion(BWTA::Region* region)
{
    for (auto it : region->getChokepoints())
        setChokepointValue(it, region, 0, true);

    DualNode* node = projectToGraph(region->getCenter());
    for (int width=15; width<=32; ++width) {
        resetChecked();
        recursiveSetChokepointValue(node, width);
    }
}

namespace
{
    bool getOrCalcChokpointBlocked(BWTA::Region* region, BWTA::Chokepoint* chokepoint, int width)
    {
        if (renewRegions.count(region) > 0) {
            setChokepointValue(chokepoint, region, 0, true);
            resetChecked();
            DualNode* node = projectToGraph(region->getCenter());
            recursiveSetChokepointValue(node, width);
        }
        auto it = chokepointWidths.find(chokepoint);
        assert( it != chokepointWidths.end() );
        int result = (chokepoint->getRegions().first == region) ? it->second.first : it->second.second;
        return (result < width);
    }
}

bool calcChokepointBlocked(BWTA::Chokepoint* chokepoint, int width)
{
    auto regions = chokepoint->getRegions();
    return getOrCalcChokpointBlocked(regions.first, chokepoint, width) || getOrCalcChokpointBlocked(regions.second, chokepoint, width);
}
