#ifndef S10HATCH_h
#define S10HATCH_h

#include "strategie.h"
#include "priorities.h"

#include "basemanagertask.h"
#include "buildneartask.h"
#include "gathermineralstask.h"
#include "gathergastask.h"
#include "overlordscouttask.h"
#include "overlordbuildtask.h"
#include "extractortricktask.h"
#include "earlyzerglingtask.h"
#include "researchtask.h"
#include "upgradetask.h"
#include "morphbuildingtask.h"

class S10HatchStrategie : public Strategie
{
public:
	Player*					me;
	BWTA::BaseLocation*		mainbase;
	BWTA::Region*			mainregion;

	BWTA::BaseLocation*		naturalbase;

	BWTA::BaseLocation*		thirdbase;

	Task*					extension_task;
	Task*					pool_task;

public:
	S10HatchStrategie(Task* tt)
		: Strategie(tt), 
		  extension_task(NULL),
		  pool_task(NULL)
	{ }

	virtual void startup()
	{
		me				= BWAPI::Broodwar->self();

		mainbase		= BWTA::getStartLocation(me); 
		mainregion		= mainbase->getRegion();

		naturalbase		= findNatural();

		thirdbase		= findThirdBase();

		// Hauptbasis verwalten:
		new BaseManagerTask(toptask, Priority::BaseManager, mainbase);

		// Overlords als Scouts verwenden:
		OverlordScoutTask* scout = new OverlordScoutTask(toptask, Priority::ScoutOverlord);
		ChokepointSet exits = mainbase->getRegion()->getChokepoints();
		if (naturalbase != NULL) {
			ChokepointSet naturalexists = naturalbase->getRegion()->getChokepoints();
			exits.insert(naturalexists.begin(), naturalexists.end());
		}
		for (ChokepointSet::iterator it=exits.begin(), itend=exits.end(); it!=itend; ++it)
			scout->points.push_back((*it)->getCenter());
		scout->checkStatus();

		// Extraktor Trick:
		if (mainbase->getGeysers().size() > 0)
			new ExtractorTrickTask(toptask, Priority::ExtractorTrick, *(mainbase->getGeysers().begin()));
	}

	virtual void computeActions(int fcount)
	{
		if (fcount % 30 != 25)
			return;

		// Extension bauen:
		static bool tryextension = true;
		if (tryextension) {
			if ((naturalbase != NULL) && (extension_task == NULL) && (me->minerals() >= 130)) {
				tryextension = false;

				extension_task = new BuildAtTask(toptask, Priority::BuildHatch, UnitTypes::Zerg_Hatchery, naturalbase->getTilePosition(), true);
				new BaseManagerTask(toptask, Priority::BaseManager, naturalbase);
			}
		}

		// Hatch 3 bauen:
		static bool trythird = true;
		if (trythird) if (!tryextension && (extension_task == NULL)) {
			trythird = false;
			if (thirdbase != NULL) {
				new BuildAtTask(toptask, Priority::BuildThirdBase, UnitTypes::Zerg_Hatchery, thirdbase->getTilePosition());
				new BaseManagerTask(toptask, Priority::BaseManager, thirdbase);
			} else {
				new BuildNearTask(toptask, Priority::BuildThirdBase, UnitTypes::Zerg_Hatchery, mainbase->getTilePosition());
			}
		}
	}

	virtual void taskChangedStatus(Task* task)
	{
		if ((task == extension_task) && (task->status == TaskStatus::Preforming)) {
			static bool first_extension_started = true;
			if (first_extension_started) {
				first_extension_started = false;
				// Overlords bauen:
				new OverlordBuildTask(toptask, Priority::BuildOverlord);

				// Pool bauen:
				pool_task = new BuildNearTask(toptask, Priority::BuildPool, UnitTypes::Zerg_Spawning_Pool, mainbase->getTilePosition());

				// Gas ernten in Main:
				BaseManagerTask* mbm = getBaseManager(mainbase);
				if (mbm != NULL)
					new GatherGasTask(mbm, Priority::GatherGas, mainbase);
			}
		}

		if ((task == extension_task) && (task->status == TaskStatus::Completed)) {
			// Gas ernten in Extension:
			BaseManagerTask* nbm = getBaseManager(naturalbase);
			if (nbm != NULL)
				new GatherGasTask(nbm, Priority::GatherGas, naturalbase);
		}

		if ((task == extension_task) && (task->status.isFinal()))
			extension_task = NULL;

		if ((task == pool_task) && (task->status == TaskStatus::Completed)) {
			//new ResearchTask(toptask, Priority::ZergSpeed, TechTypes::Infestation);
			new UpgradeTask(toptask, Priority::ZergSpeed, UpgradeTypes::Metabolic_Boost);

			// 3 mal Zergliche morphen:
			for (int k=0; k<3; ++k)
				new MorphUnitTask(toptask, Priority::FirstZerglings, UnitTypes::Zerg_Zergling);

			// 3 mal Colony an Natural:
			for (int k=0; k<3; ++k)
				new BuildNearTask(toptask, Priority::BuildColony, UnitTypes::Zerg_Creep_Colony, naturalbase->getTilePosition());

			new EarlyZerglingTask(toptask, Priority::EarlyZergling);

			new MorphBuildingTask(toptask, Priority::Lair, UnitTypes::Zerg_Lair);
		}

		if ((task == pool_task) && task->status.isFinal())
			pool_task = NULL;
	}

private:
	bool betterBase(BWTA::BaseLocation* base1, BWTA::BaseLocation* base2) const
	{
		if (base2 == NULL)
			return true;

		int gas1 = base1->getGeysers().size();
		int	gas2 = base2->getGeysers().size();
		if (gas1 > gas2)
			return true;
		if (gas1 < gas2)
			return false;

		double dis1 = BWTA::getGroundDistance(base1->getTilePosition(), mainbase->getTilePosition());
		double dis2 = BWTA::getGroundDistance(base2->getTilePosition(), mainbase->getTilePosition());
		if (dis1 < dis2)
			return true;

		return false;
	}

	BWTA::BaseLocation* findNatural() const
	{
		BWTA::Region* mainregion = mainbase->getRegion();
		RegionSet neighbours = mainregion->getReachableRegions();
		BaseLocationSet bases = mainregion->getBaseLocations();

		for (RegionSet::iterator it=neighbours.begin(), itend=neighbours.end(); it!=itend; ++it)
		{
			BWTA::Region* n = *it;
			bases.insert(n->getBaseLocations().begin(), n->getBaseLocations().end());
		}

		bases.erase(mainbase);

		BWTA::BaseLocation* best = NULL;
		for (BaseLocationSet::iterator it=bases.begin(), itend=bases.end(); it!=itend; ++it)
		{
			if (betterBase(*it, best))
				best = *it;
		}
		return best;
	}

	BWTA::BaseLocation* findThirdBase() const
	{
		BaseLocationSet bases  = mainregion->getBaseLocations();
		BaseLocationSet bases2 = naturalbase->getRegion()->getBaseLocations();
		bases.insert(bases2.begin(), bases2.end());
		
		bases.erase(mainbase);
		bases.erase(naturalbase);

		BWTA::BaseLocation* best = NULL;
		for (BaseLocationSet::iterator it=bases.begin(), itend=bases.end(); it!=itend; ++it)
		{
			if (betterBase(*it, best))
				best = *it;
		}
		return best;
	}

	BaseManagerTask* getBaseManager(BWTA::BaseLocation* loc) const
	{
		for (TaskSet::iterator it=toptask->subtasks.begin(), itend=toptask->subtasks.end(); it!=itend; ++it)
		{
			if ((*it)->getName() == BaseManagerTask::getNameStatic()) {
				BaseManagerTask* t = (BaseManagerTask*) (*it);
				if (t->base == loc)
					return t;
			}
		}
		return NULL;
	}
};

#endif