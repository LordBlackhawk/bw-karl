#include "terran-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "mineral-line.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "tech-upgrades.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include "terran-marines-code.hpp"
#include "scout.hpp"
#include "larvas.hpp"
#include "building-placer.hpp"
#include <BWTA.h>

using namespace BWAPI;
using namespace BWTA;
using namespace UnitTypes;

namespace {
	UnitPrecondition* waittill = NULL;
	bool academystarted = false;
}

bool TerranStrategieCode::isApplyable()
{
    Player* self = Broodwar->self();
    std::map<UnitType, int> units;
    for (auto it : self->getUnits())
        units[it->getType()] += 1;
    
    if ((units[Terran_SCV] < 1) || (units[Terran_Command_Center] < 1))
        return false;
    
    BaseLocation* home = getStartLocation(self);
    if (home == NULL)
        return false;
    
    if (home->getGeysers().size() < 1)
        return false;
    
    return true;
}

void TerranStrategieCode::onMatchBegin()
{
    LOG << "Standard terran opening...";

	setSupplyMode(Races::Terran, SupplyMode::Auto);
	setRequirementsMode(RequirementsMode::Auto);
 
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
        }
    }

    //useScout(getWorker(Races::Terran));
    
	for (int k=0; k<4; ++k)
		trainWorker(Terran_SCV);
	
	buildUnitEx(Terran_Supply_Depot);
	buildUnitEx(Terran_Barracks);
    
    for (int k=0; k<10; ++k)
		trainWorker(Terran_SCV);
	
    LOG << "before base planing...";
    BuildingPositionPrecondition* pos = getNextExpo(Terran_Command_Center);
    if (pos != NULL)
        useWorker(rememberFirst(buildUnit(pos, Terran_Command_Center)));
    LOG << "after base planing.";
    
	for (int k=0; k<6; ++k)
		doSomethingUsefulWithInfantry(rememberSecond(trainUnit(Terran_Marine)));
}

void TerranStrategieCode::onTick()
{
	int now = Broodwar->getFrameCount();
	if (now % 10 != 7)
		return;
	
	if (nextUnitAvailable(Terran_Barracks) < now + 500) {
		if (!academystarted) {
			buildUnitEx(Terran_Academy);
			buildRefinery(Terran_Refinery);
			researchTechEx(TechTypes::Stim_Packs);
			upgradeTechEx(UpgradeTypes::U_238_Shells);
			academystarted = true;
		}
			
		for (auto k=0; k<4; ++k)
			doSomethingUsefulWithInfantry(rememberSecond(trainUnit(Terran_Marine)));
		doSomethingUsefulWithInfantry(rememberSecond(trainUnit(Terran_Medic)));
		doSomethingUsefulWithInfantry(rememberSecond(trainUnit(Terran_Firebat)));
	} else if ((Broodwar->self()->minerals() > 400) && (buildUnitPlanSize(Terran_Barracks) < 1)) {
		buildUnitEx(Terran_Barracks);
		for (int k=0; k<3; ++k)
			trainWorker(Terran_SCV);
	}
}

void TerranStrategieCode::onMatchEnd()
{
	release(waittill);
}
