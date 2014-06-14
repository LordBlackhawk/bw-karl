#pragma once

#include "micro-task.h"
#include "informations/informations.h"
#include <BWAPI.h>

class BaseTask : public MicroTask
{
	public:
		BaseTask(MicroTaskEnum::Type t) : MicroTask(t)
		{ }
		
		TaskStatus::Type completed(UnitInfoPtr unit)
		{
			unit->popTask();
			return TaskStatus::completed;
		}

		TaskStatus::Type failed(UnitInfoPtr unit)
		{
			unit->popTask();
			return TaskStatus::failed;
		}

		int currentFrame() const
		{
			return InformationKeeper::instance().currentFrame();
		}

		int latencyFrames() const
		{
			return InformationKeeper::instance().latencyFrames();
		}

		PlayerInfoPtr self() const
		{
			return InformationKeeper::instance().self();
		}

	public:
		void deactivate(UnitInfoPtr /*u*/)
		{ }
		
		/*
		void activate(UnitInfoPtr / *u* /)
		{ }

		TaskStatus::Type tick()
		{
			return TaskStatus::failed;
		}
		*/
};