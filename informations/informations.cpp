#include "informations.h"
#include "utils/debug.h"

void InformationKeeper::clear()
{
	players.clear();
	units.clear();
	baselocations.clear();
	regions.clear();
	chokepoints.clear();
}

void InformationKeeper::prepareMap()
{
	_self    = BWAPI::Broodwar->self();
	_neutral = BWAPI::Broodwar->neutral();
	
	for (auto it : BWTA::getBaseLocations())
		getInfo(it);
		
	for (auto it : BWTA::getRegions())
		getInfo(it);
	
	for (auto it : BWTA::getChokepoints())
		getInfo(it);
}

void InformationKeeper::pretick()
{
	currentframe  = BWAPI::Broodwar->getFrameCount();
	latencyframes = BWAPI::Broodwar->getRemainingLatencyFrames();

	for (auto event : BWAPI::Broodwar->getEvents())
	{
		auto type = event.getType();
		switch (type)
		{
			case BWAPI::EventType::UnitCreate:
			case BWAPI::EventType::UnitDiscover:
				type = BWAPI::EventType::UnitShow;
				break;
			case BWAPI::EventType::UnitEvade:
				type = BWAPI::EventType::UnitHide;
				break;
			default:
				break;
		}
		switch (type)
		{
			case BWAPI::EventType::UnitMorph:
			case BWAPI::EventType::UnitShow: 
			case BWAPI::EventType::UnitHide:
			case BWAPI::EventType::UnitRenegade:
			{
				BWAPI::Unit* unit = event.getUnit();
				auto it = units.find(unit);
				if (it != units.end())
				{
					UnitInfoPtr info = getInfo(unit);
					if (type == BWAPI::EventType::UnitShow) {
						info->visible = true;
						info->readType();
						info->readOwner();
					}
					if (type == BWAPI::EventType::UnitHide)
						info->visible = false;
					if (type == BWAPI::EventType::UnitMorph)
						info->readType();
					if (type == BWAPI::EventType::UnitRenegade)
						info->readOwner();
				} else if (type == BWAPI::EventType::UnitShow) {
					if (unit->getPlayer() != _self)
						getInfo(unit);
				}
				break;
			}
			
			case BWAPI::EventType::UnitDestroy:
			{
				BWAPI::Unit* unit = event.getUnit();
				auto it = units.find(unit);
				if (it != units.end()) {
					it->second->dead    = true;
					it->second->visible = false;
					units.erase(it);
				}
				break;
			}
			
			default:
				break;
		}
	}
	
	for (auto it : units)
	{
		UnitInfoPtr info = it.second;
		info->readPosition();
	}
	
	for (auto it : baselocations)
	{
		BaseLocationInfoPtr info = it.second;
		info->readLastSeen();
	}
}

void InformationKeeper::tick()
{
	for (auto it : units)
	{
		UnitInfoPtr info = it.second;
		BWAPI::Position pos = info->lastseenPosition();
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y(), (info->isVisible() ? "%d" : "\x06%d"), info->hitPoints());
	}
	
	for (auto it : baselocations)
	{
		BaseLocationInfoPtr info = it.second;
		BWAPI::Position pos = info->pos;
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y(), (info->currentuser != NULL) ? "used!" : "free?");
	}
}

PlayerInfoPtr InformationKeeper::getInfo(BWAPI::Player* player)
{
	auto it = players.find(player);
	if (it != players.end())
		return it->second;
	
	PlayerInfoPtr info = PlayerInfoPtr(new PlayerInfo(player));
	players[player] = info;
	return info;
}

UnitInfoPtr InformationKeeper::getInfo(BWAPI::Unit* unit)
{
	auto it = units.find(unit);
	if (it != units.end())
		return it->second;
	
	UnitInfoPtr info = UnitInfoPtr(new UnitInfo(unit));
	units[unit] = info;
	return info;
}

BaseLocationInfoPtr InformationKeeper::getInfo(BWTA::BaseLocation* loc)
{
	auto it = baselocations.find(loc);
	if (it != baselocations.end())
		return it->second;
	
	BaseLocationInfoPtr info = BaseLocationInfoPtr(new BaseLocationInfo(loc));
	baselocations[loc] = info;
	return info;
}

RegionInfoPtr InformationKeeper::getInfo(BWTA::Region* region)
{
	auto it = regions.find(region);
	if (it != regions.end())
		return it->second;
	
	RegionInfoPtr info = RegionInfoPtr(new RegionInfo(region));
	regions[region] = info;
	return info;
}

ChokepointInfoPtr InformationKeeper::getInfo(BWTA::Chokepoint* point)
{
	auto it = chokepoints.find(point);
	if (it != chokepoints.end())
		return it->second;
	
	ChokepointInfoPtr info = ChokepointInfoPtr(new ChokepointInfo(point));
	chokepoints[point] = info;
	return info;
}

void InformationKeeper::baseFound(BWAPI::Unit* base)
{
	if (baselocations.empty()) {
		LOG1 << "InformationKeeper::baseFound(): baselocations are empty!!!";
		return;
	}
	
	BWAPI::Position pos = base->getPosition();
	
	double bestdis = 1e10;
	auto   bestit  = baselocations.begin();
	auto   it      = ++bestit;
	auto   itend   = baselocations.end();
	for (; it != itend; ++it)
	{
		double dis = it->second->pos.getDistance(pos);
		if (dis < bestdis) {
			bestit  = it;
			bestdis = dis;
		}
	}
	
	if (bestdis > 10*32)
		return; // Zu weit weg, als dass man es zu diese Basis zuordnen kann!
	
	BaseLocationInfoPtr info = bestit->second;
	
	if (info->currentbase != NULL) {
		if (info->currentbase->exists()) {
			double dis = info->currentbase->getPosition().getDistance(info->pos);
			if (dis < bestdis)
				return;
		}
	}
	
	info->currentbase = base;
	info->currentuser = base->getPlayer();
}

void UnitInfo::readType()
{
	BWAPI::UnitType newtype = unit->getType();
	if (newtype == type)
		return;

	type = newtype;
	if (   (type == BWAPI::UnitTypes::Zerg_Hatchery)
	    || (type == BWAPI::UnitTypes::Zerg_Lair)
		|| (type == BWAPI::UnitTypes::Zerg_Hive)
		|| (type == BWAPI::UnitTypes::Terran_Command_Center)
		|| (type == BWAPI::UnitTypes::Protoss_Nexus))
	{
		InformationKeeper::instance().baseFound(unit);
	}
}

void UnitInfo::readOwner()
{
	owner = unit->getPlayer();
}

void UnitInfo::readPosition()
{
	if (visible) {
		lastseen_time = InformationKeeper::instance().currentFrame();
		if (!isNeutral()) {
		    lastseen_pos  = unit->getPosition();
			hitpoints     = unit->getHitPoints();
		} else {
			if (lastseen_pos == BWAPI::Positions::Invalid)
				lastseen_pos  = unit->getPosition();
			hitpoints     = unit->getResources();
		}
	}
}

void BaseLocationInfo::readLastSeen()
{
	if (BWAPI::Broodwar->isVisible(tilepos)) {
		lastseen = InformationKeeper::instance().currentFrame();
	}
}