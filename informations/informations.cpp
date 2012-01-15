#include "informations.h"
#include "utils/debug.h"
#include "utils/random-chooser.h"
#include "bestway.h"
#include "micro/micro-task.h"

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
		it.second->init();
	
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
					if (type == BWAPI::EventType::UnitShow)
						info->onShow();
					if (type == BWAPI::EventType::UnitHide)
						info->onHide();
					if (type == BWAPI::EventType::UnitRenegade)
						info->onRenegade();
					if (type == BWAPI::EventType::UnitMorph)
						info->onMorph();
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
					it->second->onDeath();
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
	
	for (auto it : chokepoints)
	{
		ChokepointInfoPtr info = it.second;
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
	assert(base != NULL);
	
	if (baselocations.empty()) {
		LOG1 << "InformationKeeper::baseFound(): baselocations are empty!!!";
		return;
	}
	
	BWAPI::Position pos = base->getPosition();
	
	LOG1 << "InformationKeeper::baseFound() called for " << pos.x() << "," << pos.y() << ".";
	
	auto   bestit  = baselocations.begin();
	double bestdis = bestit->second->pos.getDistance(pos);
	auto   it      = bestit; ++it;
	auto   itend   = baselocations.end();
	for (; it!=itend; ++it)
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
	
	if (info->currentbase != NULL) {
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

void InformationKeeper::baseDestroyed(UnitInfoPtr base)
{
	for (auto it : baselocations)
		if (it.second->currentbase == base) {
			it.second->removeBase();
			return;
		}
}

void PlayerInfo::addBaseLocation(BaseLocationInfoPtr base)
{
	bases.insert(base);
	if (mainbase == NULL)
		mainbase = base;
}

void PlayerInfo::removeBaseLocation(BaseLocationInfoPtr base)
{
	bases.erase(base);
	if (mainbase == base)
		mainbase = getRandomSomething(bases);
}

void PlayerInfo::addIdleUnit(UnitInfoPtr unit)
{
	idleunits.insert(unit);
}

void PlayerInfo::removeIdleUnit(UnitInfoPtr unit)
{
	idleunits.erase(unit);
}

void PlayerInfo::addUnit(UnitInfoPtr unit)
{
	allunits.insert(unit);
}

void PlayerInfo::removeUnit(UnitInfoPtr unit)
{
	allunits.erase(unit);
}

BaseLocationInfoPtr InformationKeeper::getNearestFreeBase(const BWAPI::TilePosition& tilepos)
{
	BaseLocationInfoPtr bestbase;
	double 				bestdis = 0;
	for (auto it : baselocations)
	{
		if (it.second->currentUser() != NULL)
			continue;
		
		double dis = BWTA::getGroundDistance(it.second->getTilePosition(), tilepos);
		if ((dis < 0) || ((bestbase != NULL) && (dis > bestdis)))
			continue;
		
		bestbase = it.second;
		bestdis  = dis;
	}
	
	return bestbase;
}

BaseLocationInfoPtr PlayerInfo::getNearestFreeBase() const
{
	assert(mainbase != NULL);
	return InformationKeeper::instance().getNearestFreeBase(mainbase->getTilePosition());
}

void UnitInfo::pushTask(MicroTaskPtr task)
{
	assert(task != NULL);
	assert(owner != NULL);
	if (!tasks.empty()) {
		tasks.top()->deactivate(shared_from_this());
	} else {
		owner->removeIdleUnit(shared_from_this());
	}
	tasks.push(task);
	task->activate(shared_from_this());
}

void UnitInfo::popTask()
{
	assert(owner != NULL);
	if (!tasks.empty()) {
		tasks.top()->deactivate(shared_from_this());
		tasks.pop();
		if (!tasks.empty())
			tasks.top()->activate(shared_from_this());
		else
			owner->addIdleUnit(shared_from_this());
	}
}

void UnitInfo::popAllTasks()
{
	assert(owner != NULL);
	if (!tasks.empty()) {
		tasks.top()->deactivate(shared_from_this());
		tasks = std::stack<MicroTaskPtr>();
	} else {
		owner->removeIdleUnit(shared_from_this());
	}
}

void UnitInfo::readType()
{
	BWAPI::UnitType newtype = unit->getType();
	if (newtype == type)
		return;

	type = newtype;
	
	// Read position new for buildings!
	lastseen_pos = BWAPI::Positions::None;
	readEveryTurn();
	
	if (isBase()) {
		InformationKeeper::instance().baseFound(shared_from_this());
	} else if (type.isRefinery()) {
		readOwner();
	}
}

void UnitInfo::readOwner()
{
	BWAPI::Player* newowner = unit->getPlayer();
	if (owner != NULL) {
		if (newowner == owner->get())
			return;
		popAllTasks();
		owner->removeUnit(shared_from_this());
		owner->removeIdleUnit(shared_from_this());
	}
	
	owner = InformationKeeper::instance().getInfo(newowner);
	if (owner != NULL) {
		owner->addUnit(shared_from_this());
		owner->addIdleUnit(shared_from_this());
	}
	
	if (type == BWAPI::UnitTypes::Zerg_Extractor) {
		LOG1 << "Extractor seen, " << (owner->isNeutral() ? "is neutral" : "") << (owner->isMe() ? "is me!" : "") << "...";
	}
}

void UnitInfo::readEveryTurn()
{
	if (visible) {
		lastseen_time = InformationKeeper::instance().currentFrame();
		
		if ((lastseen_pos == BWAPI::Positions::None) || isMoveable()) {
			if (!type.isBuilding()) {
				lastseen_pos     = unit->getPosition();
				lastseen_tilepos = BWAPI::TilePosition(lastseen_pos);
			} else {
				lastseen_tilepos = unit->getTilePosition(); 
				lastseen_pos     = unit->getPosition(); //BWAPI::Position(lastseen_tilepos);
			}
		}
		
		if (!isInvincible())
			hitpoints = unit->getHitPoints();
		
		if (isResourceContainer())
			resources = unit->getResources();
	}
}

void UnitInfo::onMorph()
{
	readType();
}

void UnitInfo::onShow()
{
	visible = true;
	readOwner();
	readType();
}

void UnitInfo::onHide()
{
	visible = false;
}

void UnitInfo::onRenegade()
{
	readOwner();
}

void UnitInfo::onDeath()
{
	dead    = true;
	visible = false;
	
	if (owner != NULL) {
		popAllTasks();
		owner->removeUnit(shared_from_this());
		owner->removeIdleUnit(shared_from_this());
	}
	
	if (isBase())
		InformationKeeper::instance().baseDestroyed(shared_from_this());
}

void BaseLocationInfo::readEveryTurn()
{
	if (BWAPI::Broodwar->isVisible(tilepos))
		lastseen = InformationKeeper::instance().currentFrame();
}

void BaseLocationInfo::setNewBase(UnitInfoPtr base)
{
	if (currentuser != NULL)
		currentuser->removeBaseLocation(shared_from_this());
	
	currentbase = base;
	if (base != NULL) {
		currentuser = base->getPlayer();
	} else {
		currentuser = PlayerInfoPtr();
	}
	currentusersince = InformationKeeper::instance().currentFrame();
	
	if (currentuser != NULL)
		currentuser->addBaseLocation(shared_from_this());
}

void BaseLocationInfo::removeBase()
{
	if (currentuser != NULL)
		currentuser->removeBaseLocation(shared_from_this());
	currentbase = UnitInfoPtr();
	currentuser = PlayerInfoPtr();
	
	// TODO: Search for alternative base!
}

void ChokepointInfo::readEveryTurn()
{
	if (BWAPI::Broodwar->isVisible(tilepos))
		lastseen = InformationKeeper::instance().currentFrame();
}

void RegionInfo::init()
{
	for (auto it : region->getChokepoints())
		chokepoints.insert(InformationKeeper::instance().getInfo(it));
	
	auto it    = chokepoints.begin();
	auto itend = chokepoints.end();
	for (; it!=itend; )
		for (auto iit=++it; iit!=itend; ++iit)
	{
		std::pair<ChokepointInfoPtr, ChokepointInfoPtr> pair = (*it < *iit) ? std::make_pair(*it, *iit) : std::make_pair(*iit, *it);
		double distance = BWTA::getGroundDistance((*it)->getTilePosition(), (*iit)->getTilePosition());
		distances[pair] = distance;
	}
}