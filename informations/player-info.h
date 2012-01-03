#pragma once

#include <BWAPI.h>

class PlayerInfo
{
	friend class InformationKeeper;
	
	public:
		BWAPI::Player* get() const
		{
			return player;
		}
		
		bool isMe() const
		{
			return (player == InformationKeeper::instance().self()->get());
		}
		
		bool isNeutral() const
		{
			return (player == InformationKeeper::instance().neutral()->get());
		}
		
		bool isAlly() const
		{
			return ally;
		}
		
		bool isEnemy() const
		{
			return enemy;
		}
		
		BaseLocationInfoPtr getStartLocation() const
		{
			return startloc;
		}
		
		int getUpgradeLevel(const BWAPI::UpgradeType& gt) const
		{
			return player->getUpgradeLevel(gt);
		}
		
		bool hasResearched(const BWAPI::TechType& tt) const
		{
			return player->hasResearched(tt);
		}
		
		bool isResearching(const BWAPI::TechType& tt) const
		{
			return player->isResearching(tt);
		}
	
	protected:
		BWAPI::Player* player;
		bool ally;
		bool enemy;
		BaseLocationInfoPtr startloc;
		
		PlayerInfo(BWAPI::Player* p) : player(p)
		{
			ally     = BWAPI::Broodwar->self()->isAlly(player);
			enemy    = BWAPI::Broodwar->self()->isEnemy(player);
			startloc = InformationKeeper::instance().getInfo(BWTA::getStartLocation(player));
		}
};
