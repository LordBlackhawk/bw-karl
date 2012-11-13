#include "terran-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "mineral-line.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "tech-upgrades.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include "terran-marines-code.hpp"
#include "scout.hpp"
#include "larvas.hpp"
#include "building-placer.hpp"
#include "addon-builder.hpp"
#include "bwapi-helper.hpp"
#include "string-helper.hpp"
#include <BWTA.h>

using namespace BWAPI;
using namespace BWTA;
using namespace UnitTypes;

namespace {
    UnitPrecondition* waittill = NULL;
    bool academystarted = false;
    int  stateCounter   = 0;

    void trainAttackUnit(const UnitType& ut, const std::string& debugname = "")
    {
        doSomethingUsefulWithInfantry(rememberSecond(trainUnit(ut, debugname)));
    }
    
    void buildExpo()
    {
        BuildingPositionPrecondition* pos = getNextExpo(Terran_Command_Center);
        if (pos != NULL)
            useWorker(rememberFirst(buildUnit(pos, Terran_Command_Center)));
    }
}

bool TerranStrategieCode::isApplyable()
{
    Player* self = Broodwar->self();
    std::map<UnitType, int> units;
    for (auto it : self->getUnits())
        units[it->getType()] += 1;
    
    if ((units[Terran_SCV] < 1) || (units[Terran_Command_Center] < 1))
        return false;
    
    BaseLocation* home = getStartLocation(self);
    if (home == NULL)
        return false;
    
    if (home->getGeysers().size() < 1)
        return false;
    
    return true;
}

void TerranStrategieCode::onMatchBegin()
{
    LOG << "Standard terran opening...";
    academystarted = false;
    stateCounter = 0;

    setSupplyMode(Races::Terran, SupplyMode::Auto);
    setRequirementsMode(RequirementsMode::Auto);
 
    Player* self = Broodwar->self(); 
    for (auto it : self->getUnits()) {
        UnitType type = it->getType();
        if (type.isWorker()) {
            useWorker(it);
        } else if (type.isResourceDepot()) {
            registerBase(it);
            rememberIdle(it);
            if (type == Zerg_Hatchery)
                registerHatchery(it);
        }
    }

    for (int k=0; k<4; ++k)
        trainWorker(Terran_SCV, "Worker (Group 1)");
    
    buildUnitEx(Terran_Supply_Depot);
    buildUnitEx(Terran_Barracks);

    for (int k=0; k<5; ++k)
        trainWorker(Terran_SCV, "Worker (Group 2)");

    useScout(rememberSecond(trainUnit(Terran_SCV, "Worker (Scout)")));

    buildUnitEx(Terran_Supply_Depot);
    buildExpo();
    
    for (int k=0; k<5; ++k)
        trainWorker(Terran_SCV, "Worker (Group 3)");

    for (int k=0; k<6; ++k)
        trainAttackUnit(Terran_Marine, "Marine (Initial)");
}

void TerranStrategieCode::onTick()
{
    int now = Broodwar->getFrameCount();
    if (now % 10 != 7)
        return;
    
    if (nextUnitAvailable(Terran_Barracks) < now + 200) {
        if (!academystarted)
            buildUnitEx(Terran_Academy);

        for (auto k=0; k<4; ++k)
            trainAttackUnit(Terran_Marine, "Marine (auto)");
        trainAttackUnit(Terran_Medic, "Medic (auto)");
        trainAttackUnit(Terran_Firebat, "Firebat (auto)");

        if (!academystarted) {
            buildRefinery(Terran_Refinery);
            buildAddonEx(Terran_Comsat_Station);
            buildAddonEx(Terran_Comsat_Station);
            researchTechEx(TechTypes::Stim_Packs);
            upgradeTechEx(UpgradeTypes::U_238_Shells);
            academystarted = true;
        }
    } else if (nextUnitAvailable(Terran_Factory) < now + 200) {
        LOG << "building factory units...";
        trainAttackUnit(Terran_Siege_Tank_Tank_Mode, "Siege_Tank (auto)");
        trainAttackUnit(Terran_Vulture, "Vulture (auto)");
        trainAttackUnit(Terran_Goliath, "Goliath (auto)");
    } else if (nextUnitAvailable(Terran_Starport) < now + 200) {
        LOG << "building starport units...";
        trainAttackUnit(Terran_Wraith, "Wraith (auto)");
        trainAttackUnit(Terran_Valkyrie, "Valkyrie (auto)");
        trainAttackUnit(Terran_Dropship, "Dropship (auto)");
        trainAttackUnit(Terran_Science_Vessel, "Science Vessel (auto)");
        trainAttackUnit(Terran_Battlecruiser, "Battlecruiser (auto)");
    } else if (   (Broodwar->self()->minerals() > 400)
               && (buildUnitPlanSize(Terran_Barracks) < 1)
               && (buildUnitPlanSize(Terran_Factory)  < 1))
    {
        switch (++stateCounter)
        {
            case 1:
            case 2:
                buildUnitEx(Terran_Barracks);
                for (int k=0; k<3; ++k)
                    trainWorker(Terran_SCV, "Worker (Barracks)");
                break;
            
            case 3:
            case 4:
            case 5:
                LOG << "building factory";
                buildUnitEx(Terran_Factory);
                buildAddonEx(Terran_Machine_Shop);
                if (buildRefinery(Terran_Refinery))
                    LOG << "building additional refinery!";
                for (int k=0; k<3; ++k)
                    trainWorker(Terran_SCV, "Worker (Factory)");
                break;
            
            case 6:
                LOG << "building starport";
                buildUnitEx(Terran_Starport);
                buildAddonEx(Terran_Control_Tower);
                break;
                
            default:
                --stateCounter;
                break;
        }
    }
}

void TerranStrategieCode::onMatchEnd()
{
    release(waittill);
}

void TerranStrategieCode::onBaseMinedOut(BWTA::BaseLocation* /*base*/)
{
    LOG << "building new base...";
    buildExpo();
}

void TerranStrategieCode::onSendText(const std::string& text)
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
        if (!buildRefinery(Terran_Refinery))
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
