#pragma once

#include "utils/debug.h"

#include <BWAPI.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

class PlayerInfo;

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
		
		int lastseenTime() const
		{
			return lastseen_time;
		}
		
		BWAPI::Position getPosition() const
		{
			return lastseen_pos;
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
			assert(owner.use_count() > 0);
			return owner->isNeutral();
		}
		
		bool isMe() const
		{
			assert(owner.use_count() > 0);
			return owner->isMe();
		}
		
		bool isAlly() const
		{
			assert(owner.use_count() > 0);
			return owner->isAlly();
		}
		
		bool isEnemy() const
		{
			assert(owner.use_count() > 0);
			return owner->isEnemy();
		}
		
		bool isMoveable() const
		{
			return (type.isBuilding() && !type.isFlyer());
		}
		
		bool isInvincible() const
		{
			return type.isInvincible();
		}
		
		bool isResourceContainer() const
		{
			return type.isResourceContainer();
		}
		
		int getResources() const
		{
			return resources;
		}
		
	protected:
		void readType();	
		void readOwner();
		void readEveryTurn();
	
	protected:
		BWAPI::Unit* unit;
		
		bool dead;
		bool visible;
		BWAPI::UnitType type;
		PlayerInfoPtr owner;
		int lastseen_time;
		BWAPI::Position lastseen_pos;
		int hitpoints;
		int resources;
	
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
