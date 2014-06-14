#pragma once

#include <BWAPI.h>

class PlayerInfo
{
	friend class InformationKeeper;
	friend class BaseLocationInfo;
	friend class UnitInfo;
	
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
		
		BaseLocationInfoPtr getMainBaseLocation() const
		{
			return mainbase;
		}
		
		const std::set<BaseLocationInfoPtr>& getBases() const
		{
			return bases;
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
		
		const std::set<UnitInfoPtr>& idleUnits() const
		{
			return idleunits;
		}
		
		const std::set<UnitInfoPtr>& allUnits() const
		{
			return allunits;
		}
		
		BaseLocationInfoPtr getNearestFreeBase() const;
	
	protected:
		void addBaseLocation(BaseLocationInfoPtr base);
		void removeBaseLocation(BaseLocationInfoPtr base);
		void addUnit(UnitInfoPtr unit);
		void removeUnit(UnitInfoPtr unit);
		void addIdleUnit(UnitInfoPtr unit);
		void removeIdleUnit(UnitInfoPtr unit);
	
	protected:
		/* underlying */
		BWAPI::Player* player;
		
		/* information buffer */
		bool ally;
		bool enemy;
		BaseLocationInfoPtr mainbase;
		std::set<BaseLocationInfoPtr> bases;
		std::set<UnitInfoPtr> allunits;
		
		/* active things */
		std::set<UnitInfoPtr> idleunits;
		
		PlayerInfo(BWAPI::Player* p) : player(p)
		{
			ally  = BWAPI::Broodwar->self()->isAlly(player);
			enemy = BWAPI::Broodwar->self()->isEnemy(player);
		}
};
