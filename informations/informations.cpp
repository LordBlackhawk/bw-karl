#include "informations.h"
#include "utils/debug.h"
#include "utils/random-chooser.h"

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
	LOG1 << "loading Players/BaseLocations/Regions/Chokepoints...";
	
	_self    = getInfo(BWAPI::Broodwar->self());
	_neutral = getInfo(BWAPI::Broodwar->neutral());
	
	for (auto it : BWAPI::Broodwar->getPlayers())
		getInfo(it);
	
	for (auto it : BWTA::getBaseLocations())
		getInfo(it);
		
	for (auto it : BWTA::getRegions())
		getInfo(it);
	
	for (auto it : BWTA::getChokepoints())
		getInfo(it);
		
	for (auto it : regions)
		it->init();
	
	LOG1 << "finished loading.";
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
						info->readOwner();
						info->readType();
					}
					if (type == BWAPI::EventType::UnitHide)
						info->visible = false;
					if (type == BWAPI::EventType::UnitRenegade)
						info->readOwner();
					if (type == BWAPI::EventType::UnitMorph)
						info->readType();
				} else if (type == BWAPI::EventType::UnitShow) {
					units[unit] = UnitInfo::create(unit);
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
		info->readEveryTurn();
	}
	
	for (auto it : baselocations)
	{
		BaseLocationInfoPtr info = it.second;
		info->readEveryTurn();
	}
}

void InformationKeeper::tick()
{
	for (auto it : units)
	{
		UnitInfoPtr info = it.second;
		BWAPI::Position pos = info->getPosition();
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y(), (info->isVisible() ? "%d" : "\x06%d"), info->hitPoints());
	}
	
	for (auto it : baselocations)
	{
		BaseLocationInfoPtr info = it.second;
		BWAPI::Position pos = info->pos;
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y()+12, (info->currentuser != NULL) ? "used!" : "free?");
		BWAPI::Broodwar->drawTextMap(pos.x(), pos.y()+24, (info->isVisible() ? "visible" : "hiden"));
	}
}

PlayerInfoPtr InformationKeeper::getInfo(BWAPI::Player* player)
{
	if (player == NULL)
		return PlayerInfoPtr();

	auto it = players.find(player);
	if (it != players.end())
		return it->second;
	
	PlayerInfoPtr info = PlayerInfoPtr(new PlayerInfo(player));
	players[player] = info;
	return info;
}

UnitInfoPtr InformationKeeper::getInfo(BWAPI::Unit* unit)
{
	if (unit == NULL)
		return UnitInfoPtr();

	auto it = units.find(unit);
	if (it != units.end())
		return it->second;
	
	UnitInfoPtr info = UnitInfo::create(unit);
	units[unit] = info;
	return info;
}

BaseLocationInfoPtr InformationKeeper::getInfo(BWTA::BaseLocation* loc)
{
	if (loc == NULL)
		return BaseLocationInfoPtr();

	auto it = baselocations.find(loc);
	if (it != baselocations.end())
		return it->second;
	
	BaseLocationInfoPtr info = BaseLocationInfoPtr(new BaseLocationInfo(loc));
	baselocations[loc] = info;
	return info;
}

RegionInfoPtr InformationKeeper::getInfo(BWTA::Region* region)
{
	if (region == NULL)
		return RegionInfoPtr();

	auto it = regions.find(region);
	if (it != regions.end())
		return it->second;
	
	RegionInfoPtr info = RegionInfoPtr(new RegionInfo(region));
	regions[region] = info;
	return info;
}

ChokepointInfoPtr InformationKeeper::getInfo(BWTA::Chokepoint* point)
{
	if (point == NULL)
		return ChokepointInfoPtr();

	auto it = chokepoints.find(point);
	if (it != chokepoints.end())
		return it->second;
	
	ChokepointInfoPtr info = ChokepointInfoPtr(new ChokepointInfo(point));
	chokepoints[point] = info;
	return info;
}

void InformationKeeper::baseFound(UnitInfoPtr base)
{
	if (baselocations.empty()) {
		LOG1 << "InformationKeeper::baseFound(): baselocations are empty!!!";
		return;
	}
	
	LOG1 << "InformationKeeper::baseFound() called.";
	
	BWAPI::Position pos = base->getPosition();
	
	auto   bestit  = baselocations.begin();
	double bestdis = bestit->second->pos.getDistance(pos);
	auto   it      = bestit; ++it;
	auto   itend   = baselocations.end();
	for (; it != itend; ++it)
	{
		double dis = it->second->pos.getDistance(pos);
		if (dis < bestdis) {
			bestit  = it;
			bestdis = dis;
		}
	}
	
	if (bestdis > 10*32) {
		LOG1 << "\tbase to far away, distance is " << bestdis << "...";
		return; // Zu weit weg, als dass man es zu diese Basis zuordnen kann!
	}
	
	BaseLocationInfoPtr info = bestit->second;
	
	if (info->currentbase.use_count() > 0) {
		if (!info->currentbase->isDead()) {
			double dis = info->currentbase->getPosition().getDistance(info->pos);
			if (dis < bestdis) {
				LOG1 << "\tthere is a base more near...";
				return;
			}
		}
	}
	
	LOG1 << "\tnew base is nearer...";
	info->setNewBase(base);
}

void BaseLocationInfo::setNewBase(UnitInfoPtr base)
{
	if (currentuser.use_count() > 0)
		currentuser->removeBaseLocation(shared_from_this());
	
	currentbase = base;
	if (base.use_count() > 0) {
		currentuser = base->getPlayer();
	} else {
		currentuser = PlayerInfoPtr();
	}
	currentusersince = InformationKeeper::instance().currentFrame();
	
	if (currentuser.use_count() > 0)
		currentuser->addBaseLocation(shared_from_this());
}

void PlayerInfo::addBaseLocation(BaseLocationInfoPtr base)
{
	bases.insert(base);
	if (mainbase.use_count() == 0)
		mainbase = base;
}

void PlayerInfo::removeBaseLocation(BaseLocationInfoPtr base)
{
	bases.erase(base);
	if (mainbase == base)
		mainbase = getRandomSomething(bases);
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
		InformationKeeper::instance().baseFound(shared_from_this());
	}
}

void UnitInfo::readOwner()
{
	BWAPI::Player* newowner = unit->getPlayer();
	if (owner.use_count() > 0)
		if (newowner == owner->get())
			return;
	
	owner = InformationKeeper::instance().getInfo(newowner);
}

void UnitInfo::readEveryTurn()
{
	if (visible) {
		lastseen_time = InformationKeeper::instance().currentFrame();
		
		if ((lastseen_pos == BWAPI::Positions::Invalid) || isMoveable())
			lastseen_pos = unit->getPosition();
		
		if (!isInvincible())
			hitpoints = unit->getHitPoints();
		
		if (isResourceContainer())
			resources = unit->getResources();
	}
}

void BaseLocationInfo::readEveryTurn()
{
	if (BWAPI::Broodwar->isVisible(tilepos))
		lastseen = InformationKeeper::instance().currentFrame();

	if (currentbase != NULL)
		if (currentbase->isDead())
	{
		currentbase = UnitInfoPtr();
		currentuser = PlayerInfoPtr();
	}
}

void ChokepointInfo::readEveryTurn()
{
	if (BWAPI::Broodwar->isVisible(tilepos))
		lastseen = InformationKeeper::instance().currentFrame();
}