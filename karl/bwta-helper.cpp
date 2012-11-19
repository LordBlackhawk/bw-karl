#include "bwta-helper.hpp"

using namespace BWAPI;
using namespace BWTA;

BWTA::Region* getOtherRegion(BWTA::Chokepoint* cp, BWTA::Region* r)
{
    auto regions = cp->getRegions();
    return (regions.first == r) ? regions.second : regions.first;
}
