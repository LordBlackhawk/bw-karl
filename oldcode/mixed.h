#ifndef MIXED_h
#define MIXED_h

#include <set>
#include <list>
#include <queue>

#include "taskstatus.h"
//#include "resources.h"

#include "logfile.h"

class Agent;
typedef std::set<Agent*> AgentSet;

class Task;
typedef std::set<Task*> TaskSet;

typedef std::set<Unit*> UnitSet;

class Job;
typedef std::list<Job> JobList;

class Resources;
typedef std::set<Resources*>					ResourcesSet;

typedef std::pair<int, Agent*>					IntAgentPair;
typedef std::priority_queue<IntAgentPair> 		AgentCandidateList;

typedef std::pair<int, Task*>					IntTaskPair;
typedef std::priority_queue<IntTaskPair>		TaskCandidateList;

typedef std::pair<int, Resources*>				IntResourcesPair;
typedef std::priority_queue<IntResourcesPair>	ResourcesCandidateList;

typedef std::set<BWTA::Chokepoint*>				ChokepointSet;
typedef std::set<BWAPI::Position>				PositionSet;
typedef std::list<BWAPI::Position>				PositionList;
typedef std::set<BWTA::Region*>					RegionSet;
typedef std::set<BWTA::BaseLocation*>			BaseLocationSet;

typedef std::map<std::string, int>				StringIntMap;

#endif