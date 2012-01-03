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
		
	protected:
		void readType();	
		void readOwner();
		void readPosition();
	
	protected:
		BWAPI::Unit* unit;
		
		bool dead;
		bool visible;
		BWAPI::UnitType type;
		PlayerInfoPtr owner;
		int lastseen_time;
		BWAPI::Position lastseen_pos;
		int hitpoints; // also use for resources (Minerals, Gas)
	
	private:
		UnitInfo(BWAPI::Unit* u) : unit(u), dead(false), visible(true), lastseen_pos(BWAPI::Positions::Invalid), hitpoints(1)
		{ }
		
		void init()
		{
			readOwner();
			readPosition();
			readType();
		}
	
	protected:
		static UnitInfoPtr create(BWAPI::Unit* u)
		{
			UnitInfoPtr result = UnitInfoPtr(new UnitInfo(u));
			result->init();
			return result;
		}
};
