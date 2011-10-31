#define  NO_ASSOCIATIONS
#include "bwplan/bwplan.h"

#include <iostream>
#include <iomanip>

struct outTime
{
	int time;
	outTime(int t) : time(t) { }
	template <class Stream>
	friend Stream& operator << (Stream& stream, const outTime& data)
	{
		stream << data.time << " (" << std::setprecision(3) << ((double)data.time/15./60.) << " min)";
		return stream;
	}
};

template <class STREAM>
void outputResources(STREAM& stream, const BWResources& res)
{
	bool first = true;
	for (auto it : BWResourceIndices)
		if ((res.get(it) > 0) || (res.getLocked(it) > 0))
	{
		if (!first)
				stream << ", ";
		stream << res.get(it);
		if (it.isLockable())
			stream << "/" << res.getExisting(it);
		stream << " " << it.getName();
		first = false;
	}
	if (first)
		stream << "[There are no resources!]";
	stream << "\n";
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
		res.set<RTerranSupply>(2*10);
		res.incLocked<RTerranSupply>(0, 2*4);
	} else if (race == "Protoss") {
		res.set<RProtossWorker>(4);
		res.set<RProtossNexus>(1);
		res.set<RProtossSupply>(2*9);
		res.incLocked<RProtossSupply>(0, 2*4);
	} else if (race == "Zerg") {
		res.set<RZergWorker>(4);
		res.set<RZergHatchery>(1);
		res.set<RZergSupply>(2*9);
		res.incLocked<RZergSupply>(0, 2*4);
		res.set<RLarva>(3);
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
		if (!plan.push_back(BWOperation(index)))
			std::cerr << "Unable to add " << index.getName() << "!\n";
	}
	std::cout << "\n";

	std::cout << "Planed Operations:\n";
	for (auto it : plan.scheduledOperations())
		std::cout << "planed(" << it.scheduledTime() << "): " << it.getName() << "\n";
	std::cout << "\n";

	std::cout << "Planed Resources:\n";
	for (auto it : plan) {
		std::cout << "planed(" << it.time() << "): \t";
		outputResources(std::cout, it.getResources());
	}
	std::cout << "\n";

	std::cout << "Plan finished after " << outTime(plan.end().time()) << " frames.\n";
	return 0;
}
