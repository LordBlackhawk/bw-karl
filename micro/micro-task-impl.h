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

#define LISTTASKS		\
	DO(RegionMove)		\
	DO(LongMove)		\
	DO(LongMove)		\
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
	#define DO(name)														\
		case MicroTaskEnum::name:											\
			auto dataholder = static_pointer_cast<name##Task>(data);		\
			dataholder.activate(unit);										\
			return;

	switch (type)
	{
		LISTTASKS
		default:
			return;
	}
}

void MicroTask::deactivate(BWAPI::Unit* unit) const
{
	#define DO(name)														\
		case MicroTaskEnum::name:											\
			auto dataholder = static_pointer_cast<name##Task>(data);		\
			dataholder.deactivate(unit);									\
			return;

	switch (type)
	{
		LISTTASKS
		default:
			return;
	}
}

void MicroTask::tick(BWAPI::Unit* unit) const
{
	#define DO(name)														\
		case MicroTaskEnum::name:											\
			auto dataholder = static_pointer_cast<name##Task>(data);		\
			dataholder.tick(unit);										\
			return;

	switch (type)
	{
		LISTTAKS
		default:
			return;
	}
}

TaskStatus::Type MicroTask::status() const
{
	#define DO(name)														\
		case MicroTaskEnum::name:											\
			auto dataholder = static_pointer_cast<name##Task>(data);		\
			return dataholder.status();

	switch (type)
	{
		LISTTAKS
		default:
			return TaskStatus::failed;
	}
}

#undef DO
#undef LISTTASKS