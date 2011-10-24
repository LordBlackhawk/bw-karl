#include "BWAPI.h"
using namespace BWAPI;
#include "BWTA.h"

#include "interface.h"
#include "strategiefactory.h"

#include "threatmap.h"

#include "agentdeleter.h"
#include "agentfactory.h"
#include "agenttaskmanager.h"
#include "unitagentmap.h"

#include "s10Hatch.h"

#include "colonyagent.h"
#include "hatchagent.h"
#include "larvaagent.h"
#include "overlordagent.h"
#include "researchagent.h"
#include "workeragent.h"
#include "zerglingagent.h"

Strategie* StrategieFactory::createStrategie(Task* toptask)
{
	Race race = Broodwar->self()->getRace();
	if (race == Races::Zerg) {
		return new S10HatchStrategie(toptask);
	}
	return NULL;
}

Agent* AgentFactory::createAgent(Unit* unit)
{
	if (unit->getType().isWorker())
		return new WorkerAgent(unit);
	if (unit->getType() == UnitTypes::Zerg_Larva)
		return new LarvaAgent(unit);
	if (unit->getType() == UnitTypes::Zerg_Overlord)
		return new OverlordAgent(unit);
	if (unit->getType() == UnitTypes::Zerg_Zergling)
		return new ZerglingAgent(unit);
	if (   (unit->getType() == UnitTypes::Zerg_Creep_Colony)
		|| (unit->getType() == UnitTypes::Zerg_Sunken_Colony)
		|| (unit->getType() == UnitTypes::Zerg_Spore_Colony) )
		return new ColonyAgent(unit);
	if (   (unit->getType() == UnitTypes::Zerg_Hatchery)
		|| (unit->getType() == UnitTypes::Zerg_Lair)
		|| (unit->getType() == UnitTypes::Zerg_Hive) )
		return new HatchAgent(unit);
	if (   (unit->getType() == UnitTypes::Zerg_Spawning_Pool)
		|| (unit->getType() == UnitTypes::Zerg_Evolution_Chamber) )
		return new ResearchAgent(unit);
	Broodwar->printf("No Agent for Unittype '%s'!", unit->getType().getName().c_str());
	return NULL;
}

Task::~Task()
{
	newStatus(TaskStatus::Deleting);
	if (parent != NULL) {
		parent->subtasks.erase(this);
		parent = NULL;
	}
	while (!subtasks.empty())
		delete *subtasks.begin();  
	if (res != NULL)
		AgentTaskResourcesManager::getInstance().onResourcesUsed(res);
	reassignAgents();
}

void Task::reassignAgents()
{
	while (!agents.empty())
		AgentTaskResourcesManager::getInstance().onGiveBackAgent(*agents.begin());
}

void Task::newStatus(TaskStatus ts)
{
	if (status == ts)
		return;

	TaskStatus oldts = status;
	status           = ts;

	AgentTaskResourcesManager::getInstance().onTaskNewStatus(this, oldts, ts);
	if ((parent != NULL) && (ts != TaskStatus::Deleting))
		parent->subtaskChangedStatus(this);
}

int Task::agentDistanceToPrio(const Agent* agent, Position pos) const
{
	if (pos == Positions::None)
		return 0;

	double dis = (agent != NULL) ? agent->unit->getPosition().getDistance(pos) : 1000.0;
	return std::min(100, (int) (20000.0 / (1.0 + dis)));
}

void Task::drawTaskInfo(int sx, int& sy) const
{
	Broodwar->drawBoxScreen(sx, sy, sx+110, sy+20, Colors::Cyan);
	Broodwar->drawTextScreen(sx+3, sy+3, status.toString());
	Broodwar->drawTextScreen(sx+3, sy+3, "    : %s", getName());

	drawAgentsInfo(sx + 120, sy, agents);

	bool first = true;
	for (TaskSet::const_iterator it=subtasks.begin(), itend=subtasks.end(); it!=itend; ++it)
	{
		(*it)->drawTaskInfo(sx+120, sy);
		sy += 24;
	}
	if (subtasks.size() > 0)
		sy -= 24;
	if ((subtasks.size() == 0) && (agents.size() > 0))
		sy -= 24;
}

void Task::drawAgentsInfo(int sx, int& sy, AgentSet as) const
{
	StringIntMap units;
	for (AgentSet::const_iterator it=as.begin(), itend=as.end(); it!=itend; ++it)
		units[(*it)->unit->getType().getName()] += 1;

	for (StringIntMap::iterator it=units.begin(), itend=units.end(); it!=itend; ++it) {
		Broodwar->drawBoxScreen(sx, sy, sx+90, sy+20, Colors::Green);
		Broodwar->drawTextScreen(sx+3, sy+3, "%d %s", it->second, it->first.c_str());
		sx += 94;
	}
	if (units.size() > 0)
		sy += 24;
}

void Task::resourcesAssigned()
{
	AgentTaskResourcesManager::getInstance().onResourcesUsed(res);
}

void AgentDeleter::computeActions()
{
	AgentSet& agents = getInstance().agents;
	while (!agents.empty()) {
		Agent* agent = *(agents.begin());
		agents.erase(agent);
		Unit* unit = agent->unit;
		AgentTaskResourcesManager::getInstance().onRemoveAgent(agent);
		UnitAgentMap::remove(unit);
		delete agent;

		if (unit->exists())
		{
			Agent* agent = AgentFactory::createAgent(unit);
			if (agent != NULL) {
				UnitAgentMap::add(unit, agent);
				AgentTaskResourcesManager::getInstance().onAddAgent(agent);
			}
		}
	}
}

UnitAgentMap& UnitAgentMap::getInstance()
{
	static UnitAgentMap inst;
	return inst;
}

Interface::Interface() 
	: Task(NULL, 0), strategie(NULL)
{
	strategie = StrategieFactory::createStrategie(this);
	if (strategie != NULL)
		strategie->startup();
}

void Interface::drawTaskInfo() const
{
	int sx = 200;
	int sy = 50;

	AgentSet& allagents = AgentTaskResourcesManager::getAllAgents();
	AgentSet noowner;
	for (AgentSet::iterator it=allagents.begin(), itend=allagents.end(); it!=itend; ++it)
		if ((*it)->owner == NULL)
			noowner.insert(*it);
	drawAgentsInfo(sx, sy, noowner);

	bool first = true;
	for (TaskSet::const_iterator it=subtasks.begin(), itend=subtasks.end(); it!=itend; ++it)
	{
		(*it)->drawTaskInfo(sx, sy);
		sy += 24;
	}
}

void Interface::onFrame()
{
	try {
		int fcount = Broodwar->getFrameCount();

		ThreatMap::getInstance().onFrame(fcount);
		
		AgentDeleter::computeActions();

		if (strategie != NULL)
			strategie->computeActions(fcount);
		this->computeActions(fcount);
		AgentTaskResourcesManager::getInstance().onFrame(fcount);
	} catch (...) {
		log("Fehler in Interface::onFrame()!");
	}
}

void Interface::subtaskChangedStatus(Task* task)
{
	if (strategie != NULL)
		strategie->taskChangedStatus(task);
	Task::subtaskChangedStatus(task);
}

void Interface::onUnitCreate(BWAPI::Unit* unit)
{
	try {
		ThreatMap::getInstance().onUnitCreate(unit);

		if (UnitAgentMap::get(unit) != NULL)
			return;

		if (unit->getPlayer() == Broodwar->self()) {
			Agent* agent = AgentFactory::createAgent(unit);
			if (agent != NULL) {
				UnitAgentMap::add(unit, agent);
				AgentTaskResourcesManager::getInstance().onAddAgent(agent);
			}
		}
	} catch (...) {
		log("Fehler in Interface::onUnitCreate()!");
	}
}

void Interface::onUnitDestroy(BWAPI::Unit* unit)
{
	try {
		ThreatMap::getInstance().onUnitDestroy(unit);

		Agent* agent = UnitAgentMap::get(unit);
		if (agent != NULL) {
			if (agent->onDestroy()) {
				AgentTaskResourcesManager::getInstance().onRemoveAgent(agent);
				UnitAgentMap::remove(unit);
				delete agent;
				//Broodwar->printf("Agent removed!!!");
			} else { // Extraktor bugfix!
				UnitAgentMap::remove(unit);
				if (agent->unit != NULL)
					UnitAgentMap::add(agent->unit, agent);
				//Broodwar->printf("Agent fixed!!!");
			}
		}
	} catch (...) {
		log("Fehler in Interface::onUnitDestroy()!");
	}
}

void Interface::onUnitMorph(BWAPI::Unit* unit)
{
	try {
		ThreatMap::getInstance().onUnitMorph(unit);

		Agent* agent = UnitAgentMap::get(unit);
		if (agent != NULL) {
			if (agent->onMorph()) {
				onUnitDestroy(unit);
				onUnitCreate(unit);
			}
		}
	} catch (...) {
		log("Fehler in Interface::onUnitMorph()!");
	}
}

void Interface::onUnitRenegade(BWAPI::Unit* unit)
{
	ThreatMap::getInstance().onUnitRenegate(unit);
	onUnitDestroy(unit);
}

void Interface::onUnitDiscover(BWAPI::Unit* unit)
{
	ThreatMap::getInstance().onUnitDiscover(unit);
}

void Interface::onUnitEvade(BWAPI::Unit* unit)
{
	ThreatMap::getInstance().onUnitEvade(unit);
}

void Interface::onUnitShow(BWAPI::Unit* unit)
{
	ThreatMap::getInstance().onUnitShow(unit);
}

void Interface::onUnitHide(BWAPI::Unit* unit)
{
	ThreatMap::getInstance().onUnitHide(unit);
}

void ThreatMap::addToUpdate(UnitInformation* info)
{
	int frame = Broodwar->getFrameCount() + 50;
	update_queue.push(UnitPrioItem(frame, info));
}