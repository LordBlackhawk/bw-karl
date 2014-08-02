#pragma once

#include <BWAPI.h>

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos);
std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& pos);
