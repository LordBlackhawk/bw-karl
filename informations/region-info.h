#pragma once

#include <BWTA.h>

class RegionInfo
{
	friend class InformationKeeper;
	
	public:
		BWTA::Region* get() const
		{
			return region;
		}
	
	protected:
		BWTA::Region* region;
		
		RegionInfo(BWTA::Region* r) : region(r)
		{ }
};
