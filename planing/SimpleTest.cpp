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

DEF_RESLOCKABLE(CommandCenter)
DEF_RESLOCKABLE(TerranSupply)

DEF_RESGROWTH(Minerals, 5, TerranWorker)

typedef type_list< Minerals, CommandCenter, TerranSupply, TerranWorker > res_list;

DEF_CHECKPOINT(Build)
DEF_CHECKPOINT(BuildFinished)

BEGIN_DEF_OPTYPE(BuildTerranWorker)
	Locks<1, CommandCenter>,
	Locks<1, TerranSupply>,
	Consums<50, Minerals>,
		CheckPoint<Build, 4>,
	Prods<1, TerranWorker>,
	Unlocks<1, CommandCenter>,
		CheckPoint<BuildFinished, 0>
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
	current.advance(1);
	time = current.getTime();
}

DEF_CHECKPOINTCODE(Build)
{
	//std::cout << "Build called with (scheduledtime=" << scheduledtime << ", status=" << (int)status << ", details=" << (int)details << ")\n";
	switch (status)
	{
	case OperationStatus::started:
		std::cout << "\tResources consumt...\n";
		current.dec<Minerals>(time, 50);
		current.incLocked<TerranSupply>(time);
		current.incLocked<CommandCenter>(time);
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
		if (time > scheduledtime + 3) {
			return CheckPointResult::completed;
		} else if (time == scheduledtime + 3) {
			std::cout << "\tWorker built...\n";
			current.inc<TerranWorker>(time);
			current.decLocked<CommandCenter>(time);
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
	current.set<Minerals>		(150);
	current.set<CommandCenter>	(1);
	current.set<TerranWorker>	(2);
	current.set<TerranSupply>	(7);
	
	TestOperation op = TestOperation::get<BuildTerranWorker>();
	std::cout << "StageCount:  " << op.stageCount() << "\n";
	std::cout << "IsApplyable: " << op.isApplyable(current, 0) << "\n";
	std::cout << "Duration:    " << op.duration() << "\n";
	std::cout << "Duration(0): " << op.stageDuration(0) << "\n";
	std::cout << "Duration(1): " << op.stageDuration(1) << "\n";
	std::cout << "\n";

	TestPlan plan(current);
	for (int k=0; k<3; ++k)
		plan.push_back(TestOperation::get<BuildTerranWorker>());
		
	std::cout << "Plan.size:   " << plan.scheduledCount() << "\n";
	for (int k=0; k<plan.scheduledCount(); ++k)
		std::cout << "Scheduled("<<k<<"):" << plan.scheduled(k).scheduledTime() << "\n";
	std::cout << "\n";
	
	std::cout << "Plan complete:\n";
	for (auto it : plan) {
		std::cout << "Planed("<<it.time()<<"): \t"; outputResources(it.getResources());
	}
	std::cout << "\n";

	int counter = 0;
	while (!plan.empty() && (++counter < 15))
	{	
		plan.rebase(1, current);
		plan.execute();
		
		simulateNextRound();
		
		//std::cout << "Planed("<<time<<"): \t"; outputResources(plan.at(time).getResources());
		std::cout << "Found("<<time-1<<"):  \t"; outputResources(current);
		//std::cout << "\n";
	}
	return 0;
}