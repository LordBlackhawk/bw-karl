#ifndef WORKERAGENT_h
#define WORKERAGENT_h

#include "agent.h"

class WorkerAgent : public Agent
{
public:
	bool isZerg;

public:
	WorkerAgent(Unit* unit)
		: Agent(unit), isZerg(unit->getType().getRace() == Races::Zerg)
	{ }

	virtual void computeActions(int framecount)
	{
		// No call to Agent::computeActions()!

		switch (currentjob.type)
		{
		case Job::Reset:
			if (unit->isMorphing()) {
				unit->cancelMorph();
			} else if (unit->isConstructing()) {
				unit->cancelConstruction();
			} else {
				unit->stop();
			}
			nextJob();
			break;

		case Job::GoTo:
			if (unit->isIdle()) {
				if (unit->getDistance(currentjob.position) > 10) {
					unit->rightClick(currentjob.position);
				} else {
					nextJob();
				}
			}
			break;

		case Job::GatherMinerals:
		case Job::GatherGas:
			if (unit->isIdle()) {
				if (currentjob.target->exists()) {
					unit->rightClick(currentjob.target);
				} else {
					nextJob();
				}
			}
			break;
					
		case Job::Build:
		{
			// Zeichne geplante Position:
			UnitType ut = currentjob.unittype;
			TilePosition tp = currentjob.tileposition;
			Broodwar->drawBoxMap(tp.x()*32,tp.y()*32,tp.x()*32+ut.tileWidth()*32,tp.y()*32+ut.tileHeight()*32,Colors::Green);
			Broodwar->drawTextMap(tp.x()*32,tp.y()*32,"%s",ut.getName().c_str());

			if (unit->isIdle()) {
				if (!Broodwar->canBuildHere(unit, currentjob.tileposition, currentjob.unittype)) {
					jobFailed();
				} else {
					if (!unit->build(currentjob.tileposition, currentjob.unittype))
						unit->rightClick(Position(currentjob.tileposition));
				}
			} else if (!isZerg) {
				if (unit->getLastCommand().getType() == UnitCommandTypes::Build) {
					nextJob();
				}
			}
			break;
		}

		case Job::BuildWaiting:
			if (unit->isCompleted()) {
				currentjob = Job::Build;
				nextJob();
				markToDelete();
			}
			break;
		}
	}

	virtual bool onDestroy()
	{
		if (currentjob == Job::BuildWaiting)
			return false;

		if (   (currentjob == Job::Build) 
			&& (currentjob.unittype == UnitTypes::Zerg_Extractor) )
		{
			UnitSet units = Broodwar->getUnitsOnTile(currentjob.tileposition.x(),
								  currentjob.tileposition.y());
			for (UnitSet::iterator it=units.begin(), itend=units.end(); it!=itend; ++it)
			{
				if ((*it)->getType() == UnitTypes::Zerg_Extractor) {
					unit = *it;
					currentjob = Job::BuildWaiting;
					if (currentjob.res != NULL) {
						AgentTaskResourcesManager::getInstance().onResourcesUsed(currentjob.res);
						currentjob.res = NULL;
					}
					return false;
				}
			}
		}
		return Agent::onDestroy();
	}

	virtual bool onMorph()
	{
		if (currentjob == Job::BuildWaiting)
			return false;

		if (isZerg && (currentjob == Job::Build)) {
			currentjob = Job::BuildWaiting;
			if (currentjob.res != NULL) {
				AgentTaskResourcesManager::getInstance().onResourcesUsed(currentjob.res);
				currentjob.res = NULL;
			}
			return false;
		}

		return true;
	}
};

#endif