#pragma once

#include "bwplan.h"
#include <iomanip>

struct outTime
{
	int time;
	outTime(int t) : time(t) { }
	friend std::ostream& operator << (std::ostream& stream, const outTime& data)
	{
		stream << data.time << " (" << std::setprecision(3) << ((double)data.time/15./60.) << " min)";
		return stream;
	}
};

struct outResources
{
	const BWResources& res;
	outResources(const BWResources& r) : res(r) { }
	friend std::ostream& operator << (std::ostream& stream, const outResources& o)
	{
		const BWResources& res = o.res;
		bool first = true;
		for (auto it : BWAllResourceIndices())
			if ((res.get(it) != 0) || (it.isLockable() && (res.getLocked(it) != 0)))
		{
			if (!first)
					stream << ", ";
			stream << res.get(it);
			if (it.isLockable())
				stream << "/" << res.getExisting(it);
			stream << " " << it.getName();
			first = false;
		}
		if (first)
			stream << "[There are no resources!]";
		return stream;
	}
};