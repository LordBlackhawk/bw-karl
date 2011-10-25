#include "restypes.h"
#include "optypes.h"
#include "checkpoints.h"
#include "resources.h"
#include "operations.h"
#include "plan.h"

#include <iostream>

DEF_RESTYPE(Minerals)
DEF_RESTYPE(CommandCenter)
DEF_RESTYPE(TerranSupply)
DEF_RESTYPE(TerranWorker)

DEF_RESGROWTH(Minerals)
{
	return 5 * res.template get<TerranWorker>();
}

typedef type_list< Minerals, CommandCenter, TerranSupply, TerranWorker > res_list;

DEF_CHECKPOINT(Build)
DEF_CHECKPOINT(BuildFinished)

BEGIN_DEF_OPTYPE(BuildTerranWorker)
	Locks<1, CommandCenter>,
	Consums<50, Minerals>,
	Consums<1, TerranSupply>,
		CheckPoint<Build, 50>,
	Prods<1, TerranWorker>,
	Unlocks<1, CommandCenter>,
		CheckPoint<BuildFinished, 1>
END_DEF_OPTYPE

typedef type_list< BuildTerranWorker > op_list;

typedef PlanContainer<res_list, op_list>	TestPlan;
typedef TestPlan::ResourcesType				TestResources;
typedef TestPlan::OperationType				TestOperation;
typedef DefaultFallbackBehaviour<res_list, op_list>	TestBehaviour;

TestResources current;
int time;

void simulateNextRound()
{
	current.get<Minerals>() += 5 * current.get<TerranWorker>();
	time += 1;
}

DEF_CHECKPOINTCODE(Build)
{
	switch (status)
	{
	case OperationStatus::scheduled:
		std::cout << "\tResources consumt...\n";
		current.get<Minerals>() -= 50;
		current.get<TerranSupply>() -= 1;
		current.incLocked<CommandCenter>();
		return CheckPointResult::running;
	case OperationStatus::running:
		return CheckPointResult::completed;
	}
}

DEF_CHECKPOINTCODE(BuildFinished)
{
	switch (status)
	{
	case OperationStatus::scheduled:
	case OperationStatus::running:
		if (time >= starttime + 50) {
			std::cout << "\tWorker built...\n";
			current.get<TerranWorker>() += 1;
			current.decLocked<CommandCenter>();
			return CheckPointResult::completed;
		} else {
			return CheckPointResult::running;
		}
	}
}

void outputResources(const TestResources& res)
{
	std::cout << "Minerals: " << res.get<Minerals>() << ", "
			<< "Worker: " << res.get<TerranWorker>() << ", "
			<< "CC: " << res.get<CommandCenter>() << "/" << res.getExisting<CommandCenter>() << ", "
			<< "Supply: " << res.get<TerranSupply>() << "\n";
}

int main()
{
	time = 0;
	current.get<Minerals>() = 50;
	current.get<CommandCenter>() = 1;
	current.get<TerranWorker>() = 2;
	current.get<TerranSupply>() = 7;

	TestPlan plan(current);
	for (int k=0; k<5; ++k)
		plan.push_back(TestOperation::get<BuildTerranWorker>());

	while (!plan.empty()) {
		simulateNextRound();
		std::cout << "Planed: "; outputResources(plan.at(time).getResources());
		std::cout << "Found: "; outputResources(current);
		plan.rebase(1, current); //, exampleFallbackBehaviour);
		//plan.optimize();
		plan.execute();
	}
	return 0;
}