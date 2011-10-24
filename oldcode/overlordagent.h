#ifndef OVERLORDAGENT_h
#define OVERLORDAGENT_h

#include "agent.h"

class OverlordAgent : public Agent
{
public:
	OverlordAgent(Unit* unit) : Agent(unit)
	{
		//Broodwar->printf("OverlordAgent created.");
	}

	virtual void computeActions(int framecount)
	{
		Agent::computeActions(framecount);

		if (currentjob.type == Job::GoTo) {
			if (unit->isIdle()) {
				if (unit->getDistance(currentjob.position) > 10) {
					unit->rightClick(currentjob.position);
				} else {
					nextJob();
				}
			}
		}
	}

};

#endif