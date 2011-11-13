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

BEGIN_DEF_RESGROWTH(Minerals, 1000)
	LINEAR(45, TerranWorker)
END_DEF_RESGROWTH

DEF_CHECKPOINT(Build)
DEF_CHECKPOINT(BuildFinished)

BEGIN_DEF_OPTYPE(BuildTerranWorker)
	Locks<1, CommandCenter>,
	Locks<1, TerranSupply>,
	Consums<50, Minerals>,
		CheckPoint<Build, 4>,
	Prods<1, TerranWorker>,
	Unlocks<1, CommandCenter>,
		CheckPoint<BuildFinished, 0>,
END_DEF_OPTYPE

template <class Traits>
struct UserNames
{
	typedef ResourceIndex<Traits>	ResIndexType;
	typedef OperationIndex<Traits>	OpIndexType;
	
	static ResIndexType getResourceIndexByName(const std::string& name)
	{
		return ResIndexType::byName(name);
	}
	
	static std::string getResourceName(const ResIndexType& i)
	{
		return i.getName();
	}
	
	static OpIndexType getOperationIndexByName(const std::string& name)
	{
		return OpIndexType::byName(name);
	}
	
	static std::string getOperationName(const OpIndexType& i)
	{
		return i.getName();
	}
};

template <class Traits>
struct UserCorrection
{
	typedef PlanContainer<Traits>			PlanType;
	typedef typename PlanType::Situation	SituationType;

	static void addCorrections(PlanType& /*plan*/, const SituationType& /*it*/)
	{ }
};

struct Traits
{
	typedef TL::type_list< Minerals, CommandCenter, TerranSupply, TerranWorker > 	ResourceList;
	typedef TL::type_list< BuildTerranWorker > 										OperationList;
	typedef UserNames<Traits>														NameTraits;
	typedef UserCorrection<Traits>													CorrectionTraits;
};

typedef PlanContainer<Traits>				TestPlan;
typedef TestPlan::ResourcesType				TestResources;
typedef TestPlan::OperationType				TestOperation;
typedef TestOperation::IndexType			TestOpIndex;
typedef DefaultFallbackBehaviour<Traits>	TestBehaviour;

TestResources current;
int curtime;

void simulateNextRound()
{
	current.advance(1);
	curtime = current.getTime();
}

DEF_CHECKPOINTCODE(Build)
{
	//std::cout << "Build called with (scheduledtime=" << scheduledtime << ", status=" << (int)status << ", details=" << (int)details << ")\n";
	switch (status)
	{
	case OperationStatus::started:
		std::cout << "\tResources consumt...\n";
		current.dec<Minerals>(curtime, 50);
		current.incLocked<TerranSupply>(curtime);
		current.incLocked<CommandCenter>(curtime);
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
		if (curtime > scheduledtime + 3) {
			return CheckPointResult::completed;
		} else if (curtime == scheduledtime + 3) {
			std::cout << "\tWorker built...\n";
			current.inc<TerranWorker>(curtime);
			current.decLocked<CommandCenter>(curtime);
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
	curtime = 0;
	current.set<Minerals>		(150);
	current.set<CommandCenter>	(1);
	current.set<TerranWorker>	(2);
	current.set<TerranSupply>	(7);
	
	TestOperation op = TestOperation(TestOpIndex::byClass<BuildTerranWorker>());
	std::cout << "StageCount:  " << op.stageCount() << "\n";
	std::cout << "IsApplyable: " << op.isApplyable(current, 0) << "\n";
	std::cout << "Duration:    " << op.duration() << "\n";
	std::cout << "Duration(0): " << op.stageDuration(0) << "\n";
	std::cout << "Duration(1): " << op.stageDuration(1) << "\n";
	std::cout << "\n";

	TestPlan plan(current);
	for (int k=0; k<3; ++k)
		plan.push_back_sr(op);
		
	std::cout << "Plan.size:   " << plan.scheduledCount() << "\n";
	int k = 0;
	for (auto it : plan.scheduledOperations()) {
		std::cout << "Scheduled("<<k<<"):" << it.scheduledTime() << "\n";
		++k;
	}
	std::cout << "\n";
	
	std::cout << "Plan complete:\n";
	for (auto it : plan) {
		std::cout << "Planed("<<it.time()<<"): \t"; outputResources(it.getResources());
	}
	std::cout << "\n";

	int counter = 0;
	while (!plan.empty() && (++counter < 15))
	{	
		plan.rebase_sr(1, current);
		plan.execute();
		
		simulateNextRound();
		
		//std::cout << "Planed("<<curtime<<"): \t"; outputResources(plan.at(curtime).getResources());
		std::cout << "Found("<<curtime-1<<"):  \t"; outputResources(current);
		//std::cout << "\n";
	}
	return 0;
}