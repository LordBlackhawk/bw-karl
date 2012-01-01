#pragma once

#include "newplan/bwplan.h"
#include "informations/informations.h"

void updatePlan(BWPlan& plan)
{
	const int minPlanTime  = 1000;
	const int maxPlanTime  = 3000;
	const int minPlanItems = 3;
	
	const int minWorker    = 12;

	int starttime = plan.getStartTime();
	if ((plan.endTime()-starttime > minPlanTime) && (plan.scheduledCount() > minPlanItems))
		return;
		
	auto insert = [&] (const BWAPI::UnitType& ut) {
			LOG1 << "* Adding " << ut.getName() << ".";
			plan.push_back_sr(Operation(OperationIndex::byUnitType(ut)));
		};
	
	LOG1 << "Doing plan update...";
	
	BWAPI::UnitType ut = BWAPI::UnitTypes::Zerg_Zergling;
	
	/*
	// Techs:
	auto abilities = ut.abilities();
	for (auto it : abilities)
	{
		if (!InformationKeeper::instance().hasResearched(it)) {
			OperationIndex oi = OperationIndex::byTechType(it);
			LOG1 << "* researching " << oi.getName();
			plan.push_back_sr(Operation(oi));
			break;
		}
	}
	*/
	
	// Upgrades:
	auto upgrades = ut.upgrades();
	for (auto it : upgrades)
	{
		ResourceIndex ri = ResourceIndex::byUpgradeType(it);
		if (plan.end().getResources().get(ri) < 1) {
			OperationIndex oi = OperationIndex::byUpgradeType(it);
			LOG1 << "* upgrading " << oi.getName();
			plan.push_back_sr(Operation(oi));
			//break;
		}
	}
	
	// Arbeiter:
	if (plan.opend().getResources().get(ResourceIndex::ZergWorker) < minWorker)
		insert(BWAPI::UnitTypes::Zerg_Drone);
	
	// Zerglinge:
	while (plan.endTime()-starttime < maxPlanTime)
		insert(ut);
	
	// Hatchery:
	if (plan.opend().getResources().get(ResourceIndex::Minerals) > 300)
		insert(BWAPI::UnitTypes::Zerg_Hatchery);
	
	LOG1 << "Finished update plan.";
}