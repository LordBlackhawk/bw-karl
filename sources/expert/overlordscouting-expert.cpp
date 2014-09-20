#include "overlordscouting-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include <algorithm>

REGISTER_EXPERT(OverlordScoutingExpert)

bool OverlordScoutingExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

void OverlordScoutingExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if (port->getUnitType() != BWAPI::UnitTypes::Zerg_Overlord)
        return;
    if (port->isConnected() || port->estimatedTime > currentBlackboard->getLastUpdateTime()+100)
        return;
    overlords.push_back(port);
}


void OverlordScoutingExpert::visitResourceBoundaryItem(ResourceBoundaryItem* item){

    if(!item->getUnitType().isMineralField() || item->isVisible())
        return;
    minerals.push_back(item);
}



void OverlordScoutingExpert::endTraversal()
{
    std::vector<int> value;

    if (!minerals.empty()) {
        for(auto o:overlords) {
            for(auto m:minerals) {
                value.push_back(- 3*m->getLastSeen()  - m->getPosition().getDistance(o->getPosition()));
            }
            auto it = std::max_element(value.begin(),value.end());
            int index = it-value.begin();
            auto m = minerals[index];
			// Test-movements
			//currentBlackboard->move(o, BWAPI::Position((rand() % 500),(rand() % 500)));
            currentBlackboard->move(o, m->getTilePosition());
            value.clear();
        }
    }

    overlords.clear();
    minerals.clear();
}
