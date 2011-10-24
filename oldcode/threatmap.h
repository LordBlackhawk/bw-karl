#ifndef THREATMAP_h
#define THREATMAP_h

#include "threathelper.h"

class ThreatMap
{
public:
	static ThreatMap& getInstance()
	{
		static ThreatMap* inst = NULL;
		if (inst == NULL)
			inst = new ThreatMap();
		return *inst;
	}

private:
	ThreatMap()
		: sx(Broodwar->mapWidth()),
		  sy(Broodwar->mapHeight()),
		  ground_threat(sx, sy),
		  air_threat(sx, sy),
		  ground_force(sx, sy),
		  air_force(sx, sy),
		  tile_blocked(sx, sy)
	{
		for (int k=0; k<sx; ++k)
			for (int l=0; l<sy; ++l)
			{
				*tile_blocked.getItem(k, l) = Broodwar->isWalkable(k, l);
			}
	}

public:
	int sx, sy;
	BoolArray2d		tile_blocked;
	DoubleArray2d	ground_threat;
	DoubleArray2d	air_threat;
	DoubleArray2d	ground_force;
	DoubleArray2d	air_force;
	UnitInfoMap		unit_info_map;
	UnitPrioQueue	update_queue;

public:
	void onUnitCreate(Unit* unit)
	{
		bool groundattack = (unit->getType().groundWeapon() != WeaponTypes::None);
		bool airattack    = (unit->getType().airWeapon()    != WeaponTypes::None);

		if (!unit->getPlayer()->isNeutral() && (groundattack || airattack)) {
			UnitInformation* info = new UnitInformation(unit);
			unit_info_map[unit] = info;
			drawBlockByInfo(info, false);
			drawUnitByInfo(info, false);
			addToUpdate(info);
		}
	}

	void onUnitDestroy(Unit* unit)
	{
		UnitInfoMap::iterator it = unit_info_map.find(unit);
		if (it != unit_info_map.end()) {
			UnitInformation* info = it->second;
			drawBlockByInfo(info, true);
			drawUnitByInfo(info, true);
		}
	}

	void onUnitMorph(Unit* unit)
	{
		onUnitDiscover(unit);
	}

	void onUnitRenegate(Unit* unit)
	{
		onUnitDiscover(unit);
	}

	void onUnitDiscover(Unit* unit)
	{
		UnitInfoMap::iterator it = unit_info_map.find(unit);
		if (it != unit_info_map.end()) {
			UnitInformation* info = it->second;
			drawUnitByInfo(info, true);
			drawUnitByInfo(info, false);
		} else {
			onUnitCreate(unit);
		}
	}

	void onUnitEvade(Unit* unit)
	{
		UnitInfoMap::iterator it = unit_info_map.find(unit);
		if (it != unit_info_map.end()) {
			UnitInformation* info = it->second;
			info->lastseen = Broodwar->getFrameCount();
			drawUnitByInfo(info, true);
			drawUnitByInfo(info, false);
		}
	}

	void onUnitShow(BWAPI::Unit* unit)
	{
		onUnitDiscover(unit);
	}

	void onUnitHide(BWAPI::Unit* unit)
	{
		onUnitEvade(unit);
	}

	void onFrame(int fcount)
	{
		while (!update_queue.empty() && (update_queue.top().first <= fcount))
		{
			UnitInformation* info = update_queue.top().second;
			update_queue.pop();

			if (info->drawn)
				drawUnitByInfo(info, true);

			if (info->unit->exists()) {
				drawBlockByInfo(info, false);
				drawUnitByInfo(info, false);
				addToUpdate(info);
			} else {
				unit_info_map.erase(info->unit);
				delete info;
			}
		}
	}

public:
	bool isThreatPosition(const TilePosition& pos, bool ground) const
	{
		const DoubleArray2d& arr = (ground) ? ground_threat : air_threat;
		return (arr[pos] > 0.0);
	}

	TilePosition getGreedyFleePosition(const TilePosition& pos, bool ground) const
	{
		const DoubleArray2d& arr = (ground) ? ground_threat : air_threat;

		double basethreat = arr[pos];
		if (basethreat <= 0.0)
			return pos;

		TilePosition 	bestpos;
		double 		bestvalue = 1e10;
		for (int k=0; k<8; ++k)
		{
			TilePosition npos = getNeighbourPosition(pos, k);
			if (npos == TilePositions::None)
				continue;

			double       nthreat = arr[npos];

			if ((nthreat > basethreat) || (nthreat > bestvalue))
				continue;

			if (ground && !isMoveable(npos))
				continue;

			bestpos 	= npos;
			bestvalue 	= nthreat;
		}

		if (bestvalue >= 1e10)
			return TilePositions::Unknown;

		return bestpos;
	}

	bool isEdgePosition(const TilePosition& pos, bool ground) const
	{
		const DoubleArray2d& arr = (ground) ? ground_threat : air_threat;
		if (arr[pos] > 0.0)
			return false;

		for (int k=0; k<8; ++k)
		{
			TilePosition npos = getNeighbourPosition(pos, k);
			if (npos == TilePositions::None)
				continue;

			double nthreat = arr[npos];
			if (nthreat > 0.0)
				return true;
		}
		return false;
	}

	TilePosition getNextEdgePosition(const TilePosition& pos, bool ground, bool clockwise = true) const
	{
		const DoubleArray2d& arr = (ground) ? ground_threat : air_threat;
		if (arr[pos] > 0.0)
			return TilePositions::Unknown;

		bool fthreat[8];
		bool existsthreat = false;
		for (int k=0; k<8; ++k)
		{
			fthreat[k] = false;

			TilePosition npos = getNeighbourPosition(pos, k);
			if (npos == TilePositions::None)
				continue;

			double nthreat = arr[npos];
			if (nthreat > 0.0) {
				fthreat[k] = true;
				existsthreat = true;
			}
		}

		if (!existsthreat)
			return TilePositions::Unknown;

		int u = (clockwise) ? -1 : +1;

		int bestvalue = 0;
		int bestk;
		for (int k=0; k<8; ++k)
		{
			if (fthreat[k])
				continue;

			int value = 0;
			int l = (k+u) % 8;
			while (fthreat[l]) {
				l = (l+u) % 8;
				value++;
			}

			if (value == 0)
				continue;

			l = (l-u) % 8;
			while (!fthreat[l]) {
				l = (l-u) % 8;
				value++;
			}

			if (value < bestvalue)
				continue;

			if ((value == bestvalue) && (rand() > 0.5))
				continue;

			bestvalue = value;
			bestk     = k;
		}

		if (bestvalue == 0)
			return TilePositions::Unknown;

		return getNeighbourPosition(pos, bestk);
	}

	Path findPath(const TilePosition& from, const TilePosition& to, bool ground) const
	{	
		const DoubleArray2d& arr = (ground) ? ground_threat : air_threat;

		WayPointInfoQueue 	queue;
		TilePositionSet		visited;
		PositionPositionMap	ppmap;

		queue.push(WayPointInfo(from));

		while (!queue.empty())
		{
			WayPointInfo info(queue.top());
			queue.pop();

			if (visited.find(info.pos) != visited.end())
				continue;
			visited.insert(info.pos);

			ppmap[info.pos] = info.from;

			if (info.pos == to)
				return Path(ppmap, from, to);

			for (int k=0; k<8; ++k)
			{
				TilePosition npos = getNeighbourPosition(info.pos, k);
				if (npos == TilePositions::None)
					continue;

				if (ground && !isMoveable(npos))
					continue;

				if (visited.find(npos) != visited.end())
					continue;

				double ndis    = ((info.pos.x() != npos.x()) || (info.pos.y() != npos.y())) ? 1.0 : 1.415;
				double nthreat = arr[npos];

				queue.push(WayPointInfo(info, npos, nthreat, ndis));
			}
		}

		return Path();
	}

private:
	bool isMoveable(const TilePosition& pos) const
	{
		return !tile_blocked[pos];
	}

	TilePosition getNeighbourPosition(const TilePosition& pos, int k) const
	{
		TilePosition res = pos;
		switch (k)
		{
		case 0: res += TilePosition(1,0); break;
		case 1: res += TilePosition(1,1); break;
		case 2: res += TilePosition(0,1); break;
		case 3: res += TilePosition(-1,1); break;
		case 4: res += TilePosition(-1,0); break;
		case 5: res += TilePosition(-1,-1); break;
		case 6: res += TilePosition(0,-1); break;
		case 7: res += TilePosition(1,-1); break;
		}
		if ((res.x() < 0) || (res.x() >= sx) || (res.y() < 0) || (res.y() >= sy))
			return TilePositions::None;

		return res;
	}

	void drawUnit(DoubleArray2d& arr, const TilePosition& startpos, double value, double half_dis, double full_dis, bool ground)
	{       //                                  { value * (1 - 1/(2*half_dis) * dis)				for dis < half_dis
		// Threat Value is calculated by  = {
		//                                  { .5 * value * (dis - full_dis) / (half_dis - full_dis)		else

		PrioPositionQueue 	queue;
		TilePositionSet		visited;

		queue.push(PrioPositionItem(0.0, startpos));

		while (!queue.empty())
		{
			double dis = queue.top().first;
			TilePosition pos = queue.top().second;
			queue.pop();

			if (visited.find(pos) != visited.end())
				continue;
			visited.insert(pos);

			double calcvalue =  value * ( (dis <= half_dis) ? (1.0 - .5/half_dis * dis) : .5 * (dis - full_dis) / (half_dis - full_dis) );
			arr[pos] += calcvalue;

			for (int k=0; k<8; ++k)
			{
				TilePosition npos = getNeighbourPosition(pos, k);
				if (npos == TilePositions::None)
					continue;

				if (!isMoveable(npos) && ground)
					continue;

				if (visited.find(npos) != visited.end())
					continue;

				double ndis = dis + ((pos.x() != npos.x()) || (pos.y() != npos.y())) ? 1.0 : 1.415;

				if (ndis >= full_dis)
					continue;

				queue.push(PrioPositionItem(ndis, npos));
			}
		}
	}

	void drawUnitByInfo(UnitInformation* info, bool erase)
	{	
		Unit* unit    		= info->unit;

		UnitType type 		= (erase) ? info->lasttype : unit->getType();
		TilePosition pos 	= (erase) ? info->lastpos  : unit->getTilePosition();
		double sign 		= (erase) ? -1.0 : 1.0;
		bool ally			= (erase) ? info->lastally : unit->getPlayer()->isAlly(Broodwar->self());

		DoubleArray2d ground_arr	= (ally) ? ground_force : ground_threat;
		DoubleArray2d air_arr		= (ally) ? air_force : air_threat;

		double gvalue		= (double) type.groundWeapon().damageAmount(); 
		double ghrange      = (double) type.groundWeapon().maxRange();
		double grange		= 1.5 * ghrange;

		double avalue		= (double) type.airWeapon().damageAmount();
		double ahrange		= (double) type.airWeapon().maxRange();
		double arange		= 1.5 * ahrange;

		drawUnit(ground_arr, pos, sign * gvalue, ghrange, grange, true);
		drawUnit(air_arr, pos, sign * avalue, ahrange, arange, false);

		if (erase) {
			info->lastpos  = pos;
			info->lasttype = type;
			info->lastally = ally;
		}
	}

	void drawBlock(TilePosition pos, int sx, int sy, bool value)
	{
		for (int k=0; k<sx; ++k)
			for (int l=0; l<sy; ++l)
			{
				TilePosition npos  = pos + TilePosition(k, l);
				tile_blocked[npos] = value;
			}
	}

	void drawBlockByInfo(UnitInformation* info, bool erase)
	{
		if (erase) {
			if (info->blocking)
				drawBlock(info->lastpos, info->lasttype.tileWidth(), info->lasttype.tileHeight(), false);
			return;
		}

		if (info->blocking)
			return;

		Unit* unit 		= info->unit;
		if (unit->getType().isBuilding())
			drawBlock(unit->getTilePosition(), unit->getType().tileWidth(), unit->getType().tileHeight(), true);
	}

	void addToUpdate(UnitInformation* info);
};

#endif
