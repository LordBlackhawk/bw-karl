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
		MicroTask() : type(None)
		{ }

		MicroTask(MicroTaskEnum::Type t, const MicroTaskData& d)
			: type(t), data(d), stackedtask(p)
		{ }

		bool empty() const
		{
			return (type == None);
		}

		void activate(BWAPI::Unit* unit) const; // Hand dispatch!
		void deactivate(BWAPI::Unit* unit) const; // Hand dispatch!
		void tick(BWAPI::Unit* unit) const; // Hand dispatch!
		TaskStatus::Type status() const; // Hand dispatch!

	protected:
		MircoTaskEnum::Type		type;
		MicroTaskData			data;
};
