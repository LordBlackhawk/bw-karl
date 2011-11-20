#pragma once

#include "micro-task-manager.h"
#include <BWAPI.h>

class BaseTask
{
	public:
		BaseTask()
		{ }
		
		TaskStatus::Type completed(BWAPI::Unit* unit)
		{
			MicroTaskManager::instance().popTask(unit);
			return TaskStatus::completed;
		}

		TaskStatus::Type failed(BWAPI::Unit* unit)
		{
			MicroTaskManager::instance().popTask(unit);
			return TaskStatus::failed;
		}

		void subtask(BWAPI::Unit* unit, const MicroTask& task) const
		{
			MicroTaskManager::instance().pushTask(unit, task);
		}

		MicroTask activeTask(BWAPI::Unit* unit) const
		{
			return MicroTaskManager::instance().activeTask(unit);
		}

		int currentFrame() const
		{
			return BWAPI::Broodwar->getFrameCount();
		}

		int latencyFrames() const
		{
			return BWAPI::Broodwar->getRemainingLatencyFrames();
		}

		BWAPI::Player* self() const
		{
			return BWAPI::Broodwar->self();
		}

	public:
		void activate(BWAPI::Unit* /*u*/) const
		{ }

		void deactivate(BWAPI::Unit* /*u*/) const
		{ }

		TaskStatus::Type tick() const
		{
			return TaskStatus::failed;
		}
};