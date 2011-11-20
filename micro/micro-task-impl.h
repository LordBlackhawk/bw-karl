#pragma once

#include "micro-task.h"
#include "region-move-task.h"
#include "long-move-task.h"
#include "build-task.h"
#include "build-observer-task.h"
#include "morph-task.h"
#include "morph-observer-task.h"
#include "train-task.h"
#include "train-observer-task.h"
#include "tech-task.h"
#include "tech-observer-task.h"
#include "upgrade-task.h"
#include "upgrade-observer-task.h"

#include <cassert>

#define LISTTASKS		\
	DO(RegionMove)		\
	DO(GatherMinerals)	\
	DO(GatherGas)		\
	DO(Build)			\
	DO(BuildObserver)	\
	DO(Morph)			\
	DO(MorphObserver)	\
	DO(Train)			\
	DO(TrainObserver)	\
	DO(Tech)			\
	DO(TechObserver)	\
	DO(Upgrade)			\
	DO(UpgradeObserver)

void MicroTask::activate(BWAPI::Unit* unit) const
{
	#define DO(name) {															\
		case MicroTaskEnum::name:												\
			auto dataholder = boost::static_pointer_cast<name##Task>(data);		\
			dataholder->activate(unit);											\
			return; }

	switch (type)
	{
		LISTTASKS
		case MicroTaskEnum::None:
			return;
		default:
			std::cerr << "Unknown MicroTaskEnum: " << type << "\n";
			assert(false);
			return;
	}
	
	#undef DO
}

void MicroTask::deactivate(BWAPI::Unit* unit) const
{
	#define DO(name) {															\
		case MicroTaskEnum::name:												\
			auto dataholder = boost::static_pointer_cast<name##Task>(data);		\
			dataholder->deactivate(unit);										\
			return; }

	switch (type)
	{
		LISTTASKS
		case MicroTaskEnum::None:
			return;
		default:
			std::cerr << "Unknown MicroTaskEnum: " << type << "\n";
			assert(false);
			return;
	}
	
	#undef DO
}

TaskStatus::Type MicroTask::tick() const
{
	#define DO(name) {															\
		case MicroTaskEnum::name:												\
			auto dataholder = boost::static_pointer_cast<name##Task>(data);		\
			return dataholder->tick(); }

	switch (type)
	{
		LISTTASKS
		case MicroTaskEnum::None:
			return TaskStatus::failed;
		default:
			std::cerr << "Unknown MicroTaskEnum: " << type << "\n";
			assert(false);
			return TaskStatus::failed;
	}
	
	#undef DO
}

#undef LISTTASKS