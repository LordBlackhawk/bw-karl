#pragma once

#include "randomizer.h"
#include "newplan/bwplan.h"
#include "informations/informations.h"
#include "micro/micro-details.h"

void buildExpansion(BWPlan& plan, BWAPI::UnitType ut)
{
	Operation op(OperationIndex::byUnitType(ut));
	auto details = op.getDetails<BuildBuildingDetails>();
	assert(details != NULL);
	auto base = InformationKeeper::instance().self()->getNearestFreeBase();
	if (base != NULL) {
		details->pos = base->getTilePosition();
		details->reserve();
	}
	plan.push_back_sr(op);
}

void updatePlan(BWPlan& plan)
{
	const int minPlanTime  = 1000;
	const int maxPlanTime  = 3000;
	const int minPlanItems = 2;
	
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
	
	// Arbeiter:
	if (plan.opend().getResources().get(ResourceIndex::ZergWorker) < minWorker)
		insert(BWAPI::UnitTypes::Zerg_Drone);

	// Upgrades:
	auto upgrades = ut.upgrades();
	bool upgradesfinished = true;
	for (auto it : upgrades)
	{
		ResourceIndex  ri = ResourceIndex::byUpgradeType(it);
		OperationIndex oi = OperationIndex::byUpgradeType(it);
		if (plan.end().getResources().get(ri) < 1) {
			LOG1 << "* upgrading " << oi.getName();
			plan.push_back_sr(Operation(oi));
			upgradesfinished = false;
			break;
		}
		int current = InformationKeeper::instance().self()->getUpgradeLevel(it);
		int planed  = oi.getUpgradeLevel();
		if (current < planed) {
			LOG1 << "* descided to wait for Tech " << it.getName() << " (current: " << current << ", planed: " << planed << ").";
			upgradesfinished = false;
			break;
		}
	}
	
	// Techs:
	if (upgradesfinished) {
		auto abilities = ut.abilities();
		for (auto it : abilities)
		{
			if (InformationKeeper::instance().self()->isResearching(it)) {
				LOG1 << "* descided to wait for Tech " << it.getName();
				break;
			}
			if (!InformationKeeper::instance().self()->hasResearched(it)) {
				OperationIndex oi = OperationIndex::byTechType(it);
				LOG1 << "* researching " << oi.getName();
				plan.push_back_sr(Operation(oi));
				break;
			}
		}
	}

	// Zerglinge:
	while (plan.opEndTime()-starttime < maxPlanTime)
		insert(ut);
	
	// Hatchery:
	if (plan.end().getResources().get(ResourceIndex::ZergHatchery) < 3)
		if (plan.opend().getResources().get(ResourceIndex::Minerals) > 300) {
			//LOG1 << "[calling Randomizer::rand():]";
			//if (Randomizer::instance().rand() > .9) {
			//	insert(BWAPI::UnitTypes::Zerg_Hatchery);
			//} else {
				LOG1 << "* descided to build expansion!";
				buildExpansion(plan, BWAPI::UnitTypes::Zerg_Hatchery);
			//}
		}
	
	LOG1 << "Finished update plan.";
}