#pragma once

#include "micro-task-manager.h"
#include "worker-manager.h"

class UnitDistributer
{
	public:
		static UnitDistributer& instance()
		{
			return Singleton<UnitDistributer>::instance();
		}

		void tick() const
		{
			std::set<BWAPI::Unit*> list = MicroTaskManager::instance().inactiveUnits();
			for (auto it : list) {
				BWAPI::UnitType ut = it->getType();
				if (ut.isWorker()) {
					WorkerManager::instance().useIdleUnit(it);
				}
			}
		}
};
