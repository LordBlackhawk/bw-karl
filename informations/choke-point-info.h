#pragma once

#include <BWTA.h>

class ChokepointInfo
{
	friend class InformationKeeper;
	
	public:
		BWTA::Chokepoint* get() const
		{
			return point;
		}
	
	protected:
		BWTA::Chokepoint* point;
		
		ChokepointInfo(BWTA::Chokepoint* p) : point(p)
		{ }
};
