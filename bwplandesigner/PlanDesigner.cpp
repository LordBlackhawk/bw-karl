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
	enumerate<BWResourceList>::call<OutputInternal, const BWResources&, bool&> (res, first);
	if (first)
		std::cout << "[There are no resources!]";
	std::cout << "\n";
}

BWOperation getOperationByName(const std::string& name)
{
	BWOperation res = BWOperation::getByName(name);
	if (res.status() != OperationStatus::failed)
		return res;
	res = BWOperation::getByName("OBuildZerg_" + name);
	if (res.status() != OperationStatus::failed)
		return res;
	res = BWOperation::getByName("OBuildProtoss_" + name);
	if (res.status() != OperationStatus::failed)
		return res;
	res = BWOperation::getByName("OBuildTerran_" + name);
	if (res.status() != OperationStatus::failed)
		return res;
	return BWOperation::getByName("OBuild" + name);
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
	} else if (race == "Protoss") {
		res.set<RProtossWorker>(4);
		res.set<RProtossNexus>(1);
	} else if (race == "Zerg") {
		res.set<RZergWorker>(4);
		res.set<RZergOverlord>(1);
		res.set<RZergHatchery>(1);
	} else {
		std::cerr << "Unknown Race: " << race << "\n";
		return 1;
	}

	BWPlan plan(res, 0);
	std::cout << "Parsing arguments...\n";
	for (int k=2; k<argc; ++k) {
		BWOperation op = getOperationByName(argv[k]);
		if (op.status() == OperationStatus::failed) {
			std::cerr << "Unknown Operation Name: " << argv[k] << "\n";
			continue;
		}
		plan.push_back(op);
	}
	std::cout << "\n";

	std::cout << "Planed Operations:\n";
	for (auto it : plan.scheduled_operations())
		std::cout << "planed(" << it.scheduledTime() << "): " << it.getName() << "\n";
	std::cout << "\n";

	std::cout << "Planed Resources:\n";
	for (auto it : plan) {
		std::cout << "planed(" << it.getTime() << "): \t";
		outputResources(it.getResources());
	}
	std::cout << "\n";

	std::cout << "Plan finished after " << plan.end().getTime() << " frames.\n";
	return 0;
}
