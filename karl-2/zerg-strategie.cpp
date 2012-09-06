#include "zerg-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
#include "debugger.hpp"
#include "utils/debug.h"
#include <sstream>

using namespace BWAPI;

void ZergStrategieCode::onMatchBegin()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;

	for (int k=0; k<5; ++k) {
		std::stringstream stream;
		stream << "Unit" << k;
		UnitPrecondition* worker = morphUnit(BWAPI::UnitTypes::Zerg_Drone, stream.str().c_str());
		if (worker == NULL) {
			LOG << "Error in morphUnit(): returns NULL.";
			continue;
		}
		useWorker(worker);
	}
}

void ZergStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;
	
	if (Broodwar->getFrameCount() != 1)
		return;
	
	auto pair = buildUnit(BWAPI::UnitTypes::Zerg_Spawning_Pool);
	if (pair.first == NULL)
		LOG << "Error in buildUnit(): returns NULL.";
}
