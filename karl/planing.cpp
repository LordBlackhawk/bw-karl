#include "planing.hpp"
#include "mineral-line.hpp"
#include "larvas.hpp"
#include "idle-unit-container.hpp"
#include "supply.hpp"
#include "requirements.hpp"

using namespace BWAPI;
using namespace UnitTypes;

void initStandardPlaning(const BWAPI::Race& race)
{
    Player* self = Broodwar->self();
    for (auto it : self->getUnits()) {
        UnitType type = it->getType();
        if (type.isWorker()) {
            useWorker(it);
        } else if (type.isResourceDepot()) {
            registerBase(it);
            rememberIdle(it);
            if (type == Zerg_Hatchery)
                registerHatchery(it);
        } else if (type == Zerg_Larva) {
            registerLarva(it);
        }
    }
    
    setSupplyMode(race, SupplyMode::Auto);
    setRequirementsMode(RequirementsMode::Auto);
}
