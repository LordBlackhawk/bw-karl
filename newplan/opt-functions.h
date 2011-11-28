#pragma once

#include "plan.h"

class OptimizeEndTimeFunction
{
	public:
		bool operator () (const PlanContainer& keep, const PlanContainer& change) const
		{
			return (keep.endTime() <= change.endTime());
		}
};

