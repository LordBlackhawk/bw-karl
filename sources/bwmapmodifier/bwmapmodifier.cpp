
#include "bwmapmodifier.hpp"

int main(int /*argc*/, char** /*argv[]*/)
{
    BWMapModifier map("bwapi-data/maps/template-fight-winnable.scx");

    for(int x=-10;x<10;x++)
    {
        map.addUnit(BWAPI::UnitTypes::Terran_Marine,BWAPI::Position(32*32 + 32*x,64),0);
        map.addUnit(BWAPI::UnitTypes::Zerg_Zergling,BWAPI::Position(32*32 + 32*x,64+32),0);
        map.addUnit(BWAPI::UnitTypes::Zerg_Zergling,BWAPI::Position(32*32 + 32*x,32*64-64-32),1);
        map.addUnit(BWAPI::UnitTypes::Terran_Marine,BWAPI::Position(32*32 + 32*x,32*64-64),1);
    }
    
    map.save("bwapi-data/maps/out.scx");
    return 0;
}
