#pragma once

#include "utils/matvector.h"
#include <BWAPI.h>
#include <BWTA.h>
#include <set>

typedef MatVector<double, 2> VelocityType;

struct TaskGroupJob
{
	enum Type { move, attack, retreat, raid };
};

class TaskGroup
{
	public:
		BWAPI::Position getCurrentPosition() const;
		BWTA::Region* getCurrentRegion() const;
		double getCurrentRadius() const;
		VelocityType getCurrentVelocity() const;

		bool isAirOnly() const;
		bool isGroundOnly() const;
		bool hasTransportCapacities() const;

		double antiAirStrength() const;
		double antiGroundStrength() const;
		double hitpointsAir() const;
		double hitpointsGround() const;

		int lastSeenCompletely() const;
		int lastSeen() const;
		double visibleParts() const;

		bool isAllied() const;

		bool isFighting() const;
		std::set<TaskGroup> fightingAgainst() const;
};

class TaskGroupOwn : public TaskGroup
{
	public:
		bool isSupplyGroup() const;
		TaskGroup mainGroup() const;
		std::set<TaskGroup> getReinforceGroups() const;

		TaskGroupJob::Type getCurrentJob() const;
		BWAPI::Position getTargetPosition() const;
		BWTA::Region* getTargetRegion() const;

		void move(const BWAPI::Position& pos);
		void attack(const BWTA::Region* region);
		void retreat(const BWAPI::Position& pos);
		void raid();

		void reinforce(int value); // 0 ^= no reinforcements, 1 ^= normal, 2 ^= ...
};

double compareTaskGroup(const TaskGroup& g1, const TaskGroup& g2);