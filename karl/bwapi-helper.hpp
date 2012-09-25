#pragma once

#include <BWAPI.h>

BWAPI::Unit* getNearest(const std::set<BWAPI::Unit*>& units, const BWAPI::Position& p);
