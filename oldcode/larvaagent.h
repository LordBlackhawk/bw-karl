#ifndef LARVAAGENT_h
#define LARVAAGENT_h

#include "agent.h"
#include "morphableagent.h"

class LarvaAgent : public MorphableAgent
{
public:
	LarvaAgent(Unit* unit) : MorphableAgent(unit)
	{ }

	/*virtual void computeActions(int framecount)
	{
		if (currentjob == Job::Reset)
			nextJob();

		if (unit->getType() == UnitTypes::Zerg_Larva) {
			if (currentjob == Job::Morph) {
				//Broodwar->printf("Morphing!");
				unit->morph(currentjob.unittype);
			}
		} else if (unit->getType() != UnitTypes::Zerg_Egg) {
			if (unit->isIdle()) {
				nextJob();
				markToDelete();
				//Broodwar->printf("Larva deleted, new type is %s!", unit->getType().getName().c_str());
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
	}*/
};

#endif