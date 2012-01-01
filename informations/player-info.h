#pragma once

#include <BWAPI.h>

class PlayerInfo
{
	friend class InformationKeeper;
	
	public:
		BWAPI::Player* get() const
		{
			return player;
		}
	
	protected:
		BWAPI::Player* player;
		
		PlayerInfo(BWAPI::Player* p) : player(p)
		{ }
};
