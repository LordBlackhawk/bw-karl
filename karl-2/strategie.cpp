#include "strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "mineral-line.hpp"
#include "utils/debug.h"

void loadStrategie()
{
	for (int k=0; k<20; ++k) {
		UnitPrecondition* worker = morphUnit(BWAPI::UnitTypes::Zerg_Drone);
		if (worker == NULL) {
			LOG << "Error in morphUnit(): returns NULL.";
			continue;
		}
		useWorker(worker);
	}
}
