#ifndef HATCHAGENT_h
#define HATCHAGENT_h

#include "agent.h"
#include "morphableagent.h"
#include "baseagent.h"

class HatchAgent : public MorphableAgent
{
public:
	HatchAgent(Unit* u) : MorphableAgent(u, false)
	{
		subagent = new BaseAgent(unit);
	}

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
			if (unit->isIdle()) {
				unit->morph(currentjob.unittype);
			}
			break;

		}
	}

	virtual bool onMorph()
	{
		log("Hatch::onMorph() called!");
		if ((currentjob == Job::Morph) && (currentjob.res != NULL)) {
			AgentTaskResourcesManager::getInstance().onResourcesUsed(currentjob.res);
			currentjob.res = NULL;
		}
		return false;
	}
	*/
};

#endif