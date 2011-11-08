#pragma once

#include "bwplan.h"
#include <iomanip>

struct outTime
{
	int time;
	outTime(int t) : time(t) { }
	template <class Stream>
	friend Stream& operator << (Stream& stream, const outTime& data)
	{
		stream << data.time << " (" << std::setprecision(3) << ((double)data.time/15./60.) << " min)";
		return stream;
	}
};

struct outResources
{
	const BWResources& res;
	outResources(const BWResources& r) : res(r) { }
	template <class Stream>
	friend Stream& operator << (Stream& stream, const outResources& o)
	{
		const BWResources& res = o.res;
		bool first = true;
		for (auto it : BWResourceIndices)
			if ((res.get(it) > 0) || (it.isLockable() && (res.getLocked(it) > 0)))
		{
			if (!first)
					stream << ", ";
			stream << res.get(it);
			if (it.isLockable())
				stream << "/" << res.getExisting(it);
			stream << " " << it.getUserName();
			first = false;
		}
		if (first)
			stream << "[There are no resources!]";
		stream << "\n";
		return stream;
	}
};