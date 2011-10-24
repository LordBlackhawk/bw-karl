#ifndef THREATHELPER_h
#define THREATHELPER_h

#include "RectangleArray.h"
#include <algorithm>

template <class T> class TilePositionArray : public Util::RectangleArray<T>
{
public:
	TilePositionArray(int sx, int sy)
		: Util::RectangleArray<T>(sx, sy)
	{ }

public:
	T& operator [] (const TilePosition& pos)
	{
		return *this->getItem(pos.x(), pos.y());
	}

	const T operator [] (const TilePosition& pos) const
	{
		return this->getItem(pos.x(), pos.y());
	}
};

typedef TilePositionArray<bool>		BoolArray2d;
typedef TilePositionArray<double>	DoubleArray2d;

class UnitInformation
{
public:
	bool 			drawn;
	Unit* 			unit;
	int				lastseen;

	UnitType		lasttype;
	TilePosition 	lastpos;
	bool			lastally;

	bool			blocking;

public:
	UnitInformation(Unit* u)
		: drawn(false), unit(u)
	{ }
};

typedef std::map<Unit*, UnitInformation*>		UnitInfoMap;
typedef std::pair<int, UnitInformation*> 		UnitPrioItem;
typedef std::priority_queue<UnitPrioItem>		UnitPrioQueue;

typedef std::pair<double, TilePosition>			PrioPositionItem;
typedef std::priority_queue<PrioPositionItem>	PrioPositionQueue;
typedef std::set<TilePosition>					TilePositionSet;

typedef std::map<TilePosition, TilePosition>	PositionPositionMap;

class WayPointInfo
{
public:
	double 			value;
	TilePosition 	pos;
	TilePosition 	from;
	double			max_threat;
	double			int_threat;
	double			distance;
	
public:
	WayPointInfo(const TilePosition& f)
		: value(0.0), pos(f), from(f), max_threat(0.0), int_threat(0.0), distance(0.0)
	{ }
	
	WayPointInfo(const WayPointInfo& last, const TilePosition& p, double new_threat, double new_distance)
		: pos(p), from(last.pos)
	{
		max_threat = (last.max_threat > new_threat) ? last.max_threat : new_threat;
		int_threat = last.int_threat + new_threat;
		distance   = last.distance + new_distance;
		
		value      = .33 * max_threat + .33 * int_threat + .33 * 10.0 * distance;
	}

	bool operator < (const WayPointInfo& other) const
	{
		return (value < other.value);
	}
};

typedef std::priority_queue<WayPointInfo>		WayPointInfoQueue;

class Path
{
public:
	bool 				failed;
	std::vector<TilePosition>	waypoints;
	
	double 				max_threat;
	double 				int_threat;
	double 				distance;
	
public:
	Path() : failed(true), int_threat(0), max_threat(0)
	{ }
	
	Path(const PositionPositionMap& ppmap, const TilePosition& from, const TilePosition& to)
		: failed(false)
	{
		PositionPositionMap::const_iterator it = ppmap.find(to);
		if (it == ppmap.end()) {
			failed = true;
			return;
		}
		
		//max_threat = it->max_threat;
		//min_threat = it->min_threat;
		//distance   = it->distance;
		
		makeWayRecursive(ppmap, from, to);
		if (failed)
			waypoints.clear();
	}
	
private:
	void makeWayRecursive(const PositionPositionMap& ppmap, const TilePosition& from, const TilePosition& to)
	{
		if (from == to) {
			waypoints.push_back(from);
		} else {
			PositionPositionMap::const_iterator it = ppmap.find(to);
			if (it != ppmap.end()) {
				makeWayRecursive(ppmap, from, it->second);
				waypoints.push_back(to);
			} else {
				failed = true;
			}
		}
	}
};

#endif