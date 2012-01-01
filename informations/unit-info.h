#pragma once

#include <BWAPI.h>

class PlayerInfo;

class UnitInfo
{
	friend class InformationKeeper;
	
	public:
		BWAPI::Unit* get() const
		{
			return unit;
		}
		
		BWAPI::Player* getOwner() const
		{
			return owner;
		}
		
		int lastseenTime() const
		{
			return lastseen_time;
		}
		
		BWAPI::Position lastseenPosition() const
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
			return (owner == InformationKeeper::instance().neutral());
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
		BWAPI::Player* owner;
		int lastseen_time;
		BWAPI::Position lastseen_pos;
		int hitpoints; // also use for resources (Minerals, Gas)
		
		UnitInfo(BWAPI::Unit* u) : unit(u), dead(false), visible(true), lastseen_pos(BWAPI::Positions::Invalid), hitpoints(1)
		{
			readType();
			readOwner();
			readPosition();
		}
};
