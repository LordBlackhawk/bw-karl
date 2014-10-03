#pragma once

#include <BWAPI.h>

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos);
std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& pos);
std::ostream& operator << (std::ostream& stream, const BWAPI::EventType::Enum& type);
std::ostream& operator << (std::ostream& stream, const BWAPI::UnitType& type);
std::ostream& operator << (std::ostream& stream, const BWAPI::TechType& type);
std::ostream& operator << (std::ostream& stream, const BWAPI::UpgradeType& type);
std::ostream& operator << (std::ostream& stream, const BWAPI::Error& err);

struct BWAction
{
    enum Type { Unknown, GatheringMinerals, GatheringGas, BeingConstructed };
    static Type read(BWAPI::Unit* unit);
};
