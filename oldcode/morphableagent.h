#ifndef MORPHABLEAGENT_h
#define MORPHABLEAGENT_h

#include "agent.h"

class MorphableAgent : public Agent
{
public:
	bool newagent;

public:
	MorphableAgent(Unit* u, bool na = true) : Agent(u), newagent(na)
	{ }

	virtual void computeActions(int framecount)
	{
		Agent::computeActions(framecount);

		if (currentjob == Job::Morph) {
			if (unit->isIdle()) {
				if (unit->getType() == currentjob.unittype) {
					nextJob();
					if (newagent)
						markToDelete();
				} else {
					unit->morph(currentjob.unittype);
				}
			}
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
};

#endif