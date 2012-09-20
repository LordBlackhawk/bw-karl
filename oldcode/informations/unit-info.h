#pragma once

#include "utils/debug.h"
#include <BWAPI.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <stack>

class PlayerInfo;
class MicroTask;

typedef boost::shared_ptr<MicroTask> MicroTaskPtr;

class UnitInfo : public boost::enable_shared_from_this<UnitInfo>
{
	friend class InformationKeeper;
	
	public:
		BWAPI::Unit* get() const
		{
			return unit;
		}
		
		PlayerInfoPtr getPlayer() const
		{
			return owner;
		}
		
		BWAPI::UnitType getType() const
		{
			return type;
		}
		
		int lastseenTime() const
		{
			return lastseen_time;
		}
		
		BWAPI::Position getPosition() const
		{
			return lastseen_pos;
		}
		
		BWAPI::TilePosition getTilePosition() const
		{
			return lastseen_tilepos;
		}
		
		int hitPoints() const
		{
			return hitpoints;
		}
		
		bool isVisible() const
		{
			return visible;
		}
		
		bool isDead() const
		{
			return dead;
		}
		
		bool isNeutral() const
		{
			assert(owner != NULL);
			return owner->isNeutral();
		}
		
		bool isMe() const
		{
			assert(owner != NULL);
			return owner->isMe();
		}
		
		bool isAlly() const
		{
			assert(owner != NULL);
			return owner->isAlly();
		}
		
		bool isEnemy() const
		{
			assert(owner != NULL);
			return owner->isEnemy();
		}
		
		bool isMoveable() const
		{
			return (!type.isBuilding() && !type.isFlyingBuilding());
		}
		
		bool isInvincible() const
		{
			return type.isInvincible();
		}
		
		bool isResourceContainer() const
		{
			return type.isResourceContainer();
		}
		
		bool isBase() const
		{
			return (type == BWAPI::UnitTypes::Zerg_Hatchery)
				|| (type == BWAPI::UnitTypes::Zerg_Lair)
				|| (type == BWAPI::UnitTypes::Zerg_Hive)
				|| (type == BWAPI::UnitTypes::Terran_Command_Center)
				|| (type == BWAPI::UnitTypes::Protoss_Nexus);
		}
		
		int getResources() const
		{
			return resources;
		}
		
		MicroTaskPtr currentTask() const
		{
			return tasks.empty() ? MicroTaskPtr() : tasks.top();
		}
		
		void pushTask(MicroTaskPtr task);
		void popTask();
		void popAllTasks();
		
	protected:
		void readType();	
		void readOwner();
		void readEveryTurn();
		
		void onMorph();
		void onShow();
		void onHide();
		void onRenegade();
		void onDeath();
	
	protected:
		/* underlying */
		BWAPI::Unit* unit;
		
		/* information buffer */
		bool dead;
		bool visible;
		BWAPI::UnitType type;
		PlayerInfoPtr owner;
		int lastseen_time;
		BWAPI::Position lastseen_pos;
		BWAPI::TilePosition lastseen_tilepos;
		int hitpoints;
		int resources;
		
		/* active things */
		std::stack<MicroTaskPtr> tasks;
	
	private:
		UnitInfo(BWAPI::Unit* u)
			: unit(u), dead(false), visible(true), type(BWAPI::UnitTypes::None), lastseen_pos(BWAPI::Positions::None), hitpoints(1), resources(0)
		{ }
		
		void init()
		{
			readOwner();
			readType();
			readEveryTurn();
		}
	
	protected:
		static UnitInfoPtr create(BWAPI::Unit* u)
		{
			UnitInfoPtr result = UnitInfoPtr(new UnitInfo(u));
			result->init();
			return result;
		}
};
