#include "debugger.hpp"
#include "code-list.hpp"
#include "utils/debug.h"
#include <functional>
#include <algorithm>
#include <string>
#include <map>

namespace
{
	std::map<ResourcesPrecondition*, std::string>	resnames;
	std::map<SupplyPrecondition*, std::string>		supplynames;
	std::map<UnitPrecondition*, std::string>		unitnames;
	
	std::string defaultDebugName(void* p)
	{
		std::stringstream stream;
		stream << p;
		return stream.str();
	}
}

std::string debugName(ResourcesPrecondition* p)
{
	auto it = resnames.find(p);
	if (it == resnames.end())
		return defaultDebugName(p);
	return it->second;
}

std::string debugName(SupplyPrecondition* p)
{
	auto it = supplynames.find(p);
	if (it == supplynames.end())
		return defaultDebugName(p);
	return it->second;
}

std::string debugName(UnitPrecondition* p)
{
	auto it = unitnames.find(p);
	if (it == unitnames.end())
		return defaultDebugName(p);
	return it->second;
}

void setDebugName(ResourcesPrecondition* p, const std::string& n)
{
	resnames[p] = n;
}

void setDebugName(SupplyPrecondition* p, const std::string& n)
{
	supplynames[p] = n;
}

void setDebugName(UnitPrecondition* p, const std::string& n)
{
	unitnames[p] = n;
}

void DebuggerCode::onMatchEnd()
{
	resnames.clear();
	supplynames.clear();
	unitnames.clear();
}

void DebuggerCode::onTick()
{
	CodeList::onDebug();
}
