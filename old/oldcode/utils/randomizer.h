#pragma once

#include "utils/singleton.h"
#include <cstdlib>
#include <time.h>

class Randomizer
{
	friend class Singleton<Randomizer>;
	
	public:
		static Randomizer& instance()
		{
			return Singleton<Randomizer>::instance();
		}
		
		double rand()
		{
			return ((double)rand()/(double)RAND_MAX);
		}
		
	protected:
		Randomizer()
		{
			srand(time(NULL));
		}
};