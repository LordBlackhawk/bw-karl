#ifndef AGENTFACTORY_h
#define AGENTFACTORY_h

#include "agent.h"

class AgentFactory
{
  public:
    static Agent* createAgent(Unit* unit);
};

#endif

