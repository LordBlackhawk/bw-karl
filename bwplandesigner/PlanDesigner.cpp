#define  NO_ASSOCIATIONS
#include "bwplan/bwplan.h"

#include <iostream>

template <class RT>
struct OutputInternal
{
	static void call(const BWResources& res, bool& first)
	{
		if (res.get<RT>() > 0) {
			if (!first)
				std::cout << ", ";
			std::cout << res.get<RT>() << " " << Plan::ResourceName<RT>::name;
			first = false;
		}
	}
};

void outputResources(const BWResources& res)
{
	bool first = true;
	TL::enumerate<BWResourceList>::call<OutputInternal, const BWResources&, bool&> (res, first);
	if (first)
		std::cout << "[There are no resources!]";
	std::cout << "\n";
}

BWOperationIndex getOperationIndexByName(const std::string& name)
{
	BWOperationIndex index = BWOperationIndex::byName(name);
	if (!index.valid())
		index = BWOperationIndex::byName("OBuildZerg" + name);
	if (!index.valid())
		index = BWOperationIndex::byName("OBuildProtoss" + name);
	if (!index.valid())
		index = BWOperationIndex::byName("OBuildTerran" + name);
	if (!index.valid())
		index = BWOperationIndex::byName("OBuild" + name);
	return index;
}

int main(int argc, const char* argv[])
{
	BWResources res;
	res.set<RMinerals>(50);
	if (argc < 2) {
		std::cerr << "Use: PlanDesigner [Terran|Protoss|Zerg] [Operations...]\n";
		return 1;
	}

	std::string race = argv[1];
	if (race == "Terran") {
		res.set<RTerranWorker>(4);
		res.set<RTerranCommandCenter>(1);
		res.set<RTerranSupply>(10);
		res.incLocked<RTerranSupply>(4);
	} else if (race == "Protoss") {
		res.set<RProtossWorker>(4);
		res.set<RProtossNexus>(1);
		res.set<RProtossSupply>(9);
		res.incLocked<RProtossSupply>(4);
	} else if (race == "Zerg") {
		res.set<RZergWorker>(4);
		res.set<RZergHatchery>(1);
		res.set<RZergSupply>(9);
		res.incLocked<RZergSupply>(4);
	} else {
		std::cerr << "Unknown Race: " << race << "\n";
		return 1;
	}

	BWPlan plan(res, 0);
	std::cout << "Parsing arguments...\n";
	for (int k=2; k<argc; ++k) {
		BWOperationIndex index = getOperationIndexByName(argv[k]);
		if (!index.valid()) {
			std::cerr << "Unknown Operation Name: " << argv[k] << "\n";
			continue;
		}
		plan.push_back(BWOperation(index));
	}
	std::cout << "\n";

	std::cout << "Planed Operations:\n";
	for (auto it : plan.scheduledOperations())
		std::cout << "planed(" << it.scheduledTime() << "): " << it.getName() << "\n";
	std::cout << "\n";

	std::cout << "Planed Resources:\n";
	for (auto it : plan) {
		std::cout << "planed(" << it.time() << "): \t";
		outputResources(it.getResources());
	}
	std::cout << "\n";

	std::cout << "Plan finished after " << plan.end().time() << " frames.\n";
	return 0;
}
