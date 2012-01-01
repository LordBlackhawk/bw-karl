#pragma once

#include "plan.h"

class OptimizeEndTimeFunction
{
	public:
		bool operator () (const PlanContainer& keep, const PlanContainer& change) const
		{
			int keepA   = keep.lastOperation().scheduledEndtime();
			int keepB   = keep.beforeLastOperation().scheduledEndtime();
			int changeA = change.lastOperation().scheduledEndtime();
			int changeB = change.beforeLastOperation().scheduledEndtime();
			return ((changeA > keepA) || (changeB > keepB));
		}
};

