#ifndef AGENTDELETER_h
#define AGENTDELETER_h

class Agent;

class AgentDeleter
{
public:
	static void deleteAgent(Agent* agent)
	{
		getInstance().agents.insert(agent);
	}

	static void computeActions();

private:
	AgentDeleter()
	{ }

	AgentSet agents;

	static AgentDeleter& getInstance()
	{
		static AgentDeleter inst;
		return inst;
	}
};

#endif