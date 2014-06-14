#ifndef BASEAGENT_h
#define BASEAGENT_h

#include "agent.h"

class BaseAgent : public Agent
{
public:
	BaseAgent(Unit* unit) : Agent(unit)
	{ }
	
	virtual bool isBaseAgent() const
	{
		return true;
	}
};

#endif