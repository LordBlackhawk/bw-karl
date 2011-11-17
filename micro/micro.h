#pragma once

#include "utils/singleton.h"

class Micro
{
	public:
		static Micro& instance()
		{
			return Singleton<Micro>::instance();
		}

		void clear();
		void tick();
};
