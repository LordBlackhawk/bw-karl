#include "bw-helper.hpp"

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos)
{
    return stream << "(" << pos.x() << "," << pos.y() << ")";
}

std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& pos)
{
    return stream << "(" << pos.x() << "," << pos.y() << ")";
}
