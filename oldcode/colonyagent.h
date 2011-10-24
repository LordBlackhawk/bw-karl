#ifndef COLONYAGENT_h
#define COLONYAGENT_h

#include "agent.h"
#include "morphableagent.h"

class ColonyAgent : public MorphableAgent
{
public:
	ColonyAgent(Unit* unit) : MorphableAgent(unit)
	{ }

	/*
	virtual void computeActions(int framecount)
	{
		switch (currentjob.type)
		{
		case Job::Reset:
			nextJob();
			break;

		case Job::Waiting:
			break;

		case Job::Morph:
			if (unit->getType() != UnitTypes::Zerg_Creep_Colony) {
				nextJob();
				return;
			}
			if (unit->isIdle()) {
				unit->morph(currentjob.unittype);
			}
			break;

		}
	}

	virtual bool onMorph()
	{
		if ((currentjob == Job::Morph) && (currentjob.res != NULL)) {
			AgentTaskResourcesManager::getInstance().onResourcesUsed(currentjob.res);
			currentjob.res = NULL;
		}
		return false;
	}
	*/
};

#endif