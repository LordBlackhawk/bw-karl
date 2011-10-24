#ifndef ZERGLINGAGENT_h
#define ZERGLINGAGENT_h

#include "agent.h"

class ZerglingAgent : public Agent
{
public:
	ZerglingAgent(Unit* unit)
		: Agent(unit)
	{ }

	virtual void computeActions(int framecount)
	{
		Agent::computeActions(framecount);

		switch (currentjob.type)
		{
		case Job::ScoutPosition:
		case Job::GoTo:
			if (unit->isIdle()) {
				if (unit->getDistance(currentjob.position) > 200.0) {
					unit->rightClick(currentjob.position);
				} else {
					nextJob();
				}
			}
			break;

		case Job::AttackPosition:
			if (unit->isIdle()) {
				unit->attack(currentjob.position);
			}
			break;

		case Job::AttackNoDamage:
			if (unit->isIdle()) {
				if (   (currentjob.target != NULL)
					&& (currentjob.target->exists()))
				{
					unit->attack(currentjob.target);
				} else {
					nextJob();
				}
			}
			break;

		}
	}
};

#endif