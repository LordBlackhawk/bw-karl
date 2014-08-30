#pragma once

#include <BWAPI.h>

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos);
std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& pos);
std::ostream& operator << (std::ostream& stream, const BWAPI::EventType::Enum& type);
std::ostream& operator << (std::ostream& stream, const BWAPI::UnitType& type);
std::ostream& operator << (std::ostream& stream, const BWAPI::Error& err);
