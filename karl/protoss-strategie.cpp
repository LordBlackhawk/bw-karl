#include "protoss-strategie.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "mineral-line.hpp"
#include "idle-unit-container.hpp"
#include "tech-upgrades.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include "scout.hpp"
#include "building-placer.hpp"
#include "bwapi-helper.hpp"
#include "string-helper.hpp"
#include "terran-marines-code.hpp"
#include "planing.hpp"
#include <BWTA.h>

using namespace BWAPI;
using namespace BWTA;
using namespace UnitTypes;

namespace {
    int stateCounter = 0;
    
    void trainAttackUnit(const UnitType& ut, const std::string& debugname = "")
    {
        doSomethingUsefulWithInfantry(rememberSecond(trainUnit(ut, debugname)));
    }

    void buildExpo()
    {
        BuildingPositionPrecondition* pos = getNextExpo(Protoss_Nexus);
        if (pos != NULL)
            useWorker(rememberFirst(buildUnit(pos, Protoss_Nexus)));
    }
}

bool ProtossStrategieCode::isApplyable()
{
    Player* self = Broodwar->self();
    std::map<UnitType, int> units;
    for (auto it : self->getUnits())
        units[it->getType()] += 1;

    if ((units[Protoss_Probe] < 1) || (units[Protoss_Nexus] < 1))
        return false;

    BaseLocation* home = getStartLocation(self);
    if (home == NULL)
        return false;

    if (home->getGeysers().size() < 1)
        return false;

    return true;
}

void ProtossStrategieCode::onMatchBegin()
{
    LOG << "Standard protoss opening...";
    stateCounter = 0;

    initStandardPlaning(Races::Protoss);

    for (int k=0; k<4; ++k)
        trainWorker(Protoss_Probe, "Worker (Group 1)");
    
    buildUnitEx(Protoss_Pylon);
    buildUnitEx(Protoss_Gateway);
    
    for (int k=0; k<5; ++k)
        trainWorker(Protoss_Probe, "Worker (Group 2)");
    
    useScout(rememberSecond(trainUnit(Protoss_Probe, "Worker (Scout)")));
}

void ProtossStrategieCode::onTick()
{
    int now = Broodwar->getFrameCount();
    if (now % 10 != 7)
        return;
    
    if (nextUnitAvailable(Protoss_Gateway) < now + 200) {
        trainAttackUnit(Protoss_Zealot, "Zealot (auto)");
        if (stateCounter > 2)
            trainAttackUnit(Protoss_Dragoon, "Dragoon (auto)");
    } else if (   (Broodwar->self()->minerals() > 400)
               && (buildUnitPlanSize(Protoss_Gateway)  < 1))
    {
        switch (++stateCounter)
        {
            case 1:
            case 3:
                buildUnitEx(Protoss_Gateway);
                for (int k=0; k<5; ++k)
                    trainWorker(Protoss_Probe, "Worker (Group 3)");
                break;
            
            case 2:
                buildRefinery(Protoss_Assimilator);
                buildUnitEx(Protoss_Cybernetics_Core);
                for (int k=0; k<5; ++k)
                    trainWorker(Protoss_Probe, "Worker (Group 3)");
                break;
            
            case 4:
                buildUnitEx(Protoss_Forge);
                for (int k=0; k<3; ++k)
                    buildUnitEx(Protoss_Photon_Cannon);
                break;
            
            case 5:
                buildExpo();
                break;
            
            default:
                --stateCounter;
                break;
        }
    }
}

void ProtossStrategieCode::onMatchEnd()
{ }

void ProtossStrategieCode::onBaseMinedOut(BWTA::BaseLocation* /*base*/)
{
    LOG << "building new base...";
    buildExpo();
}

void ProtossStrategieCode::onSendText(const std::string& text)
{
    std::vector<std::string> words = splitIntoWords(text);
    if (words[0] != "/build")
        return;

    if (words.size() < 2) {
        Broodwar->printf("The build command needs the unit type as argument.");
        return;
    }

    if (words[1] == "expo") {
        buildExpo();
    } else if (words[1] == "refinery") {
        if (!buildRefinery(Protoss_Assimilator))
            Broodwar->printf("No position for refinery found.");
    } else {
        UnitType ut = getUnitTypeByName(words[1]);
        if (ut == UnitTypes::Unknown) {
            Broodwar->printf("Unknown unit type: %s", words[1].c_str());
            return;
        }

        rememberIdle(createUnit(ut));
    }
}
