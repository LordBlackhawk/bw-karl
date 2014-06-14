#pragma once

#include "timetype.h"
#include "resources.h"

struct LinearCorrection
{
	ResourceIndex	ri;
	TimeInterval 	interval;
	int          	value;

	LinearCorrection() : value(0)
	{ }
	
	LinearCorrection(const ResourceIndex& ri_, const TimeInterval& i_, const int& v_) : ri(ri_), interval(i_), value(v_)
	{ }

	void apply(Resources& res, const TimeInterval& range) const
	{
		TimeInterval local = interval ^ range;
		if (!local.empty())
			res.incInternal(ri, value * local.length());
	}

	bool isLaterAs(const TimeType& time) const
	{
		return (time <= interval.upper);
	}
};