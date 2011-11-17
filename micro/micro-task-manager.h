#pragma once

#include "micro-task.h"
#include "utils/singleton.h"
#include <BWAPI.h>
#include <map>

struct MicroTaskStack;
typedef boost::shared_ptr<MicroTaskStack> MicroTaskStackPtr;

struct MicroTaskStack
{
	MicroTask			task;
	MicroTaskStackPtr	next;
	MicroTaskStack(const MicroTask& t, const MicroTaskStackPtr& s = MicroTaskStackPtr()) : task(t), next(s)
	{ }
};

class MicroTaskManager
{
	public:
		static MicroTaskManager& instance()
		{
			return Singleton<MicroTaskManager>::instance();
		}

		void pushTask(BWAPI::Unit* unit, const MicroTask& task)
		{
			auto it = activeunits.find(unit);
			if (it == activeunits.end()) {
				inactiveunits.erase(unit);
				MicroTaskStackPtr element = MicroTaskStackPtr(new MicroTaskStack(task));
				activeunits[unit] = element;
			} else {
				it->second->task.deactivate(unit);
				it->second = MicroTaskStackPtr(new MicroTaskStack(task, it->second));
			}
			task.activate(unit);
		}

		void popTask(BWAPI::Unit* unit)
		{
			auto it = activeunits.find(unit);
			if (it == activeunits.end())
				return;
			it->second->task.deactivate(unit);
			if (it->second->next.use_count() == 0) {
				activeunits.erase(it);
				if (unit->exists())
					inactiveunits.insert(unit);
				return;
			}
			it->second = it->second->next;
			it->second->task.activate(unit);
		}

		MicroTask activeTask(BWAPI::Unit* unit) const
		{
			auto it = activeunits.find(unit);
			if (it == activeunits.end())
				return MicroTask();
			return it->second->task;
		}

		void onUnitAdded(BWAPI::Unit* unit)
		{
			inactiveunits.insert(unit);
		}

		void onUnitDestroyed(BWAPI::Unit* unit)
		{
			auto it = activeunits.find(unit);
			if (it != activeunits.end()) {
				it->second->task.deactivate(unit);
				activeunits.erase(it);
			} else {
				inacitveunits.erase(unit);
			}
		}

		void onTick()
		{
			for (auto it : activeunits)
				it->second->task->tick(it->first);
		}

		std::set<BWAPI::Unit*> inactiveUnits() const
		{
			return inactiveunits;
		}

		std::set<BWAPI::Unit*> allUnits() const
		{
			std::set<BWAPI::Unit*> result = inactiveunits;
			for (auto it : activeunits)
				result.insert(it.first);
			return result;
		}

		void clear()
		{
			activeunits.clear();
			inactiveunits.clear();
		}
	protected:
		std::map<BWAPI::Unit*, MicroTaskStackPtr>	activeunits;
		std::set<BWAPI::Unit*>						inactiveunits;
};