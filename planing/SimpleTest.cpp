#include <iostream>

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
		CheckPoint<Build, 4>,
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
	//std::cout << "Build called with (scheduledtime=" << scheduledtime << ", status=" << (int)status << ", details=" << (int)details << ")\n";
	switch (status)
	{
	case OperationStatus::started:
		//std::cout << "\tResources consumt...\n";
		current.get<Minerals>() -= 50;
		current.get<TerranSupply>() -= 1;
		current.incLocked<CommandCenter>();
		return CheckPointResult::running;
	case OperationStatus::running:
		return CheckPointResult::completed;
	default:
		return CheckPointResult::waiting;
	}
}

DEF_CHECKPOINTCODE(BuildFinished)
{
	//std::cout << "BuildFinished called with (scheduledtime=" << scheduledtime << ", status=" << (int)status << ", details=" << (int)details << ")\n";
	switch (status)
	{
	case OperationStatus::started:
	case OperationStatus::running:
		if (time > scheduledtime + 4) {
			return CheckPointResult::completed;
		} else if (time == scheduledtime + 4) {
			//std::cout << "\tWorker built...\n";
			current.get<TerranWorker>() += 1;
			current.decLocked<CommandCenter>();
		}
		return CheckPointResult::running;
	default:
		return CheckPointResult::waiting;
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
	current.get<Minerals>() 		= 150;
	current.get<CommandCenter>() 	= 1;
	current.get<TerranWorker>() 	= 2;
	current.get<TerranSupply>() 	= 7;
	
	TestOperation op = TestOperation::get<BuildTerranWorker>();
	std::cout << "StageCount:  " << op.stageCount() << "\n";
	std::cout << "IsApplyable: " << op.isApplyable(current, 0) << "\n";
	std::cout << "Duration:    " << op.duration() << "\n";
	std::cout << "Duration(0): " << op.stageDuration(0) << "\n";
	std::cout << "Duration(1): " << op.stageDuration(1) << "\n";
	std::cout << "\n";

	TestPlan plan(current);
	for (int k=0; k<5; ++k)
		plan.push_back(TestOperation::get<BuildTerranWorker>());
		
	std::cout << "Plan.size:   " << plan.scheduledCount() << "\n";
	for (int k=0; k<plan.scheduledCount(); ++k)
		std::cout << "Scheduled("<<k<<"):" << plan.scheduled(k).scheduledTime() << "\n";
	std::cout << "\n";

	int counter = 0;
	while (!plan.empty() && (++counter < 15))
	{
		simulateNextRound();
		std::cout << "Planed: \t"; outputResources(plan.at(time).getResources());
		plan.rebase(1, current);
		plan.execute();
		std::cout << "Found:  \t"; outputResources(current);
		std::cout << "\n";
	}
	return 0;
}