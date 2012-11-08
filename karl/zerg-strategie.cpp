#include "zerg-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "precondition-helper.hpp"
#include "building-placer.hpp"
#include "larvas.hpp"
#include "log.hpp"
#include <BWTA.h>
#include <sstream>

using namespace BWAPI;
using namespace BWTA;
using namespace UnitTypes;

namespace {
	Precondition* waittill = NULL;
}

bool ZergStrategieCode::isApplyable()
{
    Player* self = Broodwar->self();
    std::map<UnitType, int> units;
    for (auto it : self->getUnits())
        units[it->getType()] += 1;
    
    if ((units[Zerg_Drone] < 1) || (units[Zerg_Hatchery] < 1) || (units[Zerg_Overlord] < 1))
        return false;
    
    BWTA::BaseLocation* home = BWTA::getStartLocation(self);
    if (home == NULL)
        return false;
    
    if (home->getGeysers().size() < 1)
        return false;
    
    return true;
}

void ZergStrategieCode::onMatchBegin()
{
    LOG << "Standard zerg opening...";

	setSupplyMode(Races::Zerg, SupplyMode::Auto);
	setRequirementsMode(RequirementsMode::Auto);
    
    Player* self = Broodwar->self();
    for (auto it : self->getUnits()) {
        UnitType type = it->getType();
        if (type.isWorker()) {
            useWorker(it);
        } else if (type.isResourceDepot()) {
            registerBase(it);
            if (type == Zerg_Hatchery)
                registerHatchery(it);
            rememberIdle(it);
        } else if (type == Zerg_Larva) {
            registerLarva(it);
        }
    }

	for (int k=0; k<5; ++k)
		morphWorker(Zerg_Drone);
	morphUnitEx(Zerg_Overlord);
	buildUnitEx(Zerg_Spawning_Pool);
	
	for (int k=0; k<5; ++k)
		morphWorker(Zerg_Drone);
	buildRefinery(Zerg_Extractor);
	
    BuildingPositionPrecondition* pos = getNextExpo(Zerg_Hatchery);
    if (pos != NULL) {
        UnitPrecondition* hatch = buildUnit(pos, Zerg_Hatchery).first;
        waittill = hatch;
    }
	
	UnitPrecondition* colony = buildUnit(Zerg_Creep_Colony).first;
	rememberIdle(morphUnit(colony, Zerg_Sunken_Colony));
	
	for (int k=0; k<15; ++k)
		morphWorker(Zerg_Drone);
	
	for (int k=0; k<10; ++k)
		morphUnitEx(Zerg_Zergling);
}

void ZergStrategieCode::onTick()
{
	if ((waittill == NULL) || (waittill->time == 0)) {
		release(waittill);
		
		for (int k=0; k<9; ++k)
			morphUnitEx(Zerg_Zergling);
            
        for (int k=0; k<3; ++k)
            morphUnitEx(Zerg_Mutalisk);
		
		waittill = morphUnit(Zerg_Zergling);
	}
}

void ZergStrategieCode::onMatchEnd()
{
	release(waittill);
}
