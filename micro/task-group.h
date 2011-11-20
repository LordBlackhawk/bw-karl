#pragma once

#include <BWAPI.h>
#include <BWTA.h>

struct TaskGroupEnum
{
	enum Type { waiting, };
};

struct TaskGroupCommand
{
	enum Type { wait, attack, defend };
	BWTA::Region* region;
	
};

class TaskGroup
{
	public:
		TaskGroup() : type(waiting)
		{ }
		
		
	
	protected:
		TaskGroupEnum::Type 	type;
		std::set<BWAPI::Unit*>	units;
		std::set<
	
};