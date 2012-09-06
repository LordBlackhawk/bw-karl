#include "strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "mineral-line.hpp"
#include "debugger.hpp"
#include "utils/debug.h"
#include <sstream>

void loadStrategie()
{
	for (int k=0; k<20; ++k) {
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
