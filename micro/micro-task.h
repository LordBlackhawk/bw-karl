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

		void activate(BWAPI::Unit* unit) const; // Hand dispatch!
		void deactivate(BWAPI::Unit* unit) const; // Hand dispatch!
		void tick(BWAPI::Unit* unit) const; // Hand dispatch!
		TaskStatus::Type status() const; // Hand dispatch!

	protected:
		MicroTaskEnum::Type		type;
		MicroTaskData			data;
};
