#pragma once

#include "micro-task-manager.h"
#include <BWAPI.h>

class BaseTask
{
	public:
		BaseTask() : status_(TaskStatus::running)
		{ }
		
		void completed(BWAPI::Unit* unit) const
		{
			status_ = TaskStatus::completed;
			MicroTaskManager::instance().popTask(unit);
		}

		void failed(BWAPI::Unit* unit) const
		{
			status_ = TaskStatus::failed;
			MicroTaskManager::instance().popTask(unit);
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
			return Broodwar->getFrameCount();
		}

		int latencyFrames() const
		{
			return Broodwar->getRemainingLatencyFrames
		}

		BWAPI::Player* self() const
		{
			return Broodwar->self();
		}

	public:
		void activate(BWAPI::Unit* u) const
		{ }

		void deactivate(BWAPI::Unit* u) const
		{ }

		void tick(BWAPI::Unit* u) const
		{ }

		TaskStatus::Type status() const
		{
			return status_;
		}

	protected:
		TaskStatus::Type status_;
};