#pragma once

#include "micro-task-enum.h"
#include <BWAPI.h>
#include <boost/shared_ptr.hpp>

struct TaskStatus
{
	enum Type { running, completed, failed };
};

typedef boost::shared_ptr<void>			MicroTaskData;

class MicroTask
{
	public:
		MicroTask()
			: type(MicroTaskEnum::None)
		{ }

		MicroTask(MicroTaskEnum::Type t, const MicroTaskData& d)
			: type(t), data(d)
		{ }

		bool empty() const
		{
			return (type == MicroTaskEnum::None);
		}
		
		bool isGatherMinerals() const
		{
			return (type == MicroTaskEnum::GatherMinerals);
		}

		void activate(BWAPI::Unit* unit) const; // Hand dispatch!
		void deactivate(BWAPI::Unit* unit) const; // Hand dispatch!
		TaskStatus::Type tick() const; // Hand dispatch!
		
		bool operator < (const MicroTask& other) const
		{
			return (data < other.data);
		}
		
		bool operator == (const MicroTask& other) const
		{
			return (data == other.data);
		}

	protected:
		MicroTaskEnum::Type		type;
		MicroTaskData			data;
};
