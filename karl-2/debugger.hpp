#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

std::string debugName(ResourcesPrecondition* p);
std::string debugName(SupplyPrecondition* p);
std::string debugName(UnitPrecondition* p);

void setDebugName(ResourcesPrecondition* p, const std::string& n);
void setDebugName(SupplyPrecondition* p, const std::string& n);
void setDebugName(UnitPrecondition* p, const std::string& n);

struct DebuggerCode : public DefaultCode
{
	static void onMatchEnd();
	static void onTick();
};
