#pragma once

#include "micro-task-enum.h"
#include "informations/informations.h"
#include <BWAPI.h>
#include <boost/shared_ptr.hpp>

struct TaskStatus
{
	enum Type { running, completed, failed };
};

class MicroTask
{
	public:
		MicroTask(MicroTaskEnum::Type t)
			: type(t)
		{ }

		bool empty() const
		{
			return (type == MicroTaskEnum::None);
		}
		
		bool isGatherMinerals() const
		{
			return (type == MicroTaskEnum::GatherMinerals);
		}

		virtual void activate(UnitInfoPtr unit) = 0;
		virtual void deactivate(UnitInfoPtr unit) = 0;
		virtual TaskStatus::Type tick() = 0;

	protected:
		MicroTaskEnum::Type		type;
};
