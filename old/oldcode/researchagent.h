#ifndef RESEARCHAGENT_h
#define RESEARCHAGENT_h

#include "agent.h"

class ResearchAgent : public Agent
{
public:
	ResearchAgent(Unit* u) : Agent(u)
	{ }

	virtual void computeActions(int framecount)
	{
		Agent::computeActions(framecount);

		if (currentjob == Job::Research) {
			if (unit->isIdle()) {
				Player* me = Broodwar->self();
				if (   me->hasResearched(currentjob.techtype)
					|| me->isResearching(currentjob.techtype)) {
					nextJob();
				} else {
					unit->research(currentjob.techtype);
				}
			} else if ((currentjob.res != NULL) && unit->isResearching() && (unit->getTech() == currentjob.techtype)) {
				AgentTaskResourcesManager::getInstance().onResourcesUsed(currentjob.res);
				currentjob.res = NULL;
			}
		} else if (currentjob == Job::Upgrade) {
			if (unit->isIdle()) {
				Player* me = Broodwar->self();
				if (   (me->getUpgradeLevel(currentjob.upgradetype) > 0)
					|| me->isUpgrading(currentjob.upgradetype)) {
					nextJob();
				} else {
					unit->upgrade(currentjob.upgradetype);
				}
			} else if ((currentjob.res != NULL) && unit->isUpgrading() && (unit->getUpgrade() == currentjob.upgradetype)) {
				AgentTaskResourcesManager::getInstance().onResourcesUsed(currentjob.res);
				currentjob.res = NULL;
			}
		}
	}

};

#endif