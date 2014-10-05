
#include "bwmapmodifier.hpp"

int main(int /*argc*/, char** /*argv[]*/)
{
    BWMapModifier map("bwapi-data/maps/template-fight-winnable.scx");

    for(int x=-10;x<10;x++)
    {
        //map.addUnit(BWAPI::UnitTypes::Terran_Marine,BWAPI::Position(32*32 + 32*x,64),0);
        map.addUnit(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode,BWAPI::Position(32*32 + 32*x,64),0);
        map.addUnit(BWAPI::UnitTypes::Zerg_Zergling,BWAPI::Position(32*32 + 32*x,64+32),0);
        map.addUnit(BWAPI::UnitTypes::Zerg_Zergling,BWAPI::Position(32*32 + 32*x,32*64-64-32),1);
        //map.addUnit(BWAPI::UnitTypes::Terran_Marine,BWAPI::Position(32*32 + 32*x,32*64-64),1);
        map.addUnit(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode,BWAPI::Position(32*32 + 32*x,32*64-64),1);
    }

    map.setOwner(0,OwnerSectionInterpreter::OwnerType::Human_Open);
    map.setOwner(1,OwnerSectionInterpreter::OwnerType::Computer);
    map.setOwner(2,OwnerSectionInterpreter::OwnerType::Inactive);
    map.setOwner(3,OwnerSectionInterpreter::OwnerType::Inactive);
    map.setOwner(4,OwnerSectionInterpreter::OwnerType::Inactive);
    map.setOwner(5,OwnerSectionInterpreter::OwnerType::Inactive);
    map.setOwner(6,OwnerSectionInterpreter::OwnerType::Inactive);
    map.setOwner(7,OwnerSectionInterpreter::OwnerType::Inactive);

    map.setRace(0,RaceSectionInterpreter::RaceType::Zerg);
    map.setRace(1,RaceSectionInterpreter::RaceType::Terran);
    map.setRace(2,RaceSectionInterpreter::RaceType::Protoss);
    map.setRace(3,RaceSectionInterpreter::RaceType::User_Selectable);

    map.setUpgrade(-1,BWAPI::UpgradeTypes::Zerg_Carapace,1,3);
    map.setUpgrade(0,BWAPI::UpgradeTypes::Zerg_Carapace,3,3);

    map.setTech(-1,BWAPI::TechTypes::Tank_Siege_Mode,1,1);

    map.save("bwapi-data/maps/out.scx");
    return 0;
}
