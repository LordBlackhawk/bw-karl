#include "bw-helper.hpp"
#include <ostream>

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos)
{
    if (pos == BWAPI::Positions::Unknown)
        return stream << "(Unknown)";
    if (pos == BWAPI::Positions::Invalid)
        return stream << "(Invalid)";
    if (pos == BWAPI::Positions::None)
        return stream << "(None)";
    return stream << "(" << pos.x() << "," << pos.y() << ")";
}

std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& pos)
{
    if (pos == BWAPI::TilePositions::Unknown)
        return stream << "(Unknown)";
    if (pos == BWAPI::TilePositions::Invalid)
        return stream << "(Invalid)";
    if (pos == BWAPI::TilePositions::None)
        return stream << "(None)";
    return stream << "(" << pos.x() << "," << pos.y() << ")";
}

std::ostream& operator << (std::ostream& stream, const BWAPI::EventType::Enum& type)
{
    #define DO(name) case BWAPI::EventType::name: return stream << #name;
    switch (type)
    {
        DO(MatchStart)
        DO(MatchEnd)
        DO(MatchFrame)
        DO(MenuFrame)
        DO(SendText)
        DO(ReceiveText)
        DO(PlayerLeft)
        DO(NukeDetect)
        DO(UnitDiscover)
        DO(UnitEvade)
        DO(UnitShow)
        DO(UnitHide)
        DO(UnitCreate)
        DO(UnitDestroy)
        DO(UnitMorph)
        DO(UnitRenegade)
        DO(SaveGame)
        DO(UnitComplete)
        DO(PlayerDropped)
        DO(None)
        default:
            return stream << "unknown event type";
    }
}

std::ostream& operator << (std::ostream& stream, const BWAPI::UnitType& type)
{
    return stream << type.c_str();
}

std::ostream& operator << (std::ostream& stream, const BWAPI::Error& err)
{
    return stream << err.toString();
}
