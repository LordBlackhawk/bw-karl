#pragma once

#include "planing/resourceindex.h"
#include "planing/operationindex.h"

template <class Traits>
struct BWUserNames
{
	typedef ResourceIndex<Traits>	ResIndexType;
	typedef OperationIndex<Traits>	OpIndexType;
	
	static ResIndexType getResourceIndexByName(const std::string& name)
	{
		ResIndexType index = ResIndexType::byName(name);
		if (index.valid())
			return index;
		index = ResIndexType::byName("RZerg" + name);
		if (index.valid())
			return index;
		index = ResIndexType::byName("RProtoss" + name);
		if (index.valid())
			return index;
		index = ResIndexType::byName("RTerran" + name);
		if (index.valid())
			return index;
		return ResIndexType::byName("R" + name);
	}
	
	static OpIndexType getOperationIndexByName(const std::string& name)
	{
		OpIndexType index = OpIndexType::byName(name);
		if (!index.valid())
			index = OpIndexType::byName("OBuildZerg" + name);
		if (!index.valid())
			index = OpIndexType::byName("OBuildProtoss" + name);
		if (!index.valid())
			index = OpIndexType::byName("OBuildTerran" + name);
		if (!index.valid())
			index = OpIndexType::byName("OBuild" + name);
		return index;
	}
};
