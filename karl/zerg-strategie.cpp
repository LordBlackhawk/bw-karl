#include "zerg-strategie.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "larvas.hpp"
#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "building-placer.hpp"
#include "tech-upgrades.hpp"
#include "bwapi-helper.hpp"
#include "planing.hpp"
#include "log.hpp"
#include "valuing.hpp"
#include "scout.hpp"
#include <BWTA.h>
#include <sstream>

using namespace BWAPI;
using namespace BWTA;
using namespace UnitTypes;
using namespace UpgradeTypes;

namespace {
    const int timehorizon      = 300;

    void morphAttackUnit(const UnitType& type)
    {
        morphUnitEx(type);
    }

    void buildExpo()
    {
        BuildingPositionPrecondition* pos = getNextExpo(Zerg_Hatchery);
        if (pos != NULL)
            rememberIdle(buildUnit(pos, Zerg_Hatchery).first);
    }

    UpgradeType zergling_gts[] = { Metabolic_Boost,
                                   Zerg_Carapace,
                                   Zerg_Melee_Attacks,
                                   Zerg_Carapace,
                                   Zerg_Melee_Attacks,
                                   Adrenal_Glands,
                                   Zerg_Carapace,
                                   Zerg_Melee_Attacks
                                 };
    int zergling_gt_level    = 0;
    int zergling_gt_maxlevel = sizeof(zergling_gts) / sizeof(UpgradeType);

    UpgradeType nextZerglingUpgrade()
    {
        if (zergling_gt_level >= zergling_gt_maxlevel)
            return UpgradeTypes::Unknown;
        UpgradeType result = zergling_gts[zergling_gt_level];
        ++zergling_gt_level;
        return result;
    }
}

bool ZergStrategieCode::isApplyable()
{
    Player* self = Broodwar->self();
    std::map<UnitType, int> units;
    for (auto it : self->getUnits())
        units[it->getType()] += 1;
    
    if ((units[Zerg_Drone] < 1) || (units[Zerg_Hatchery] < 1) || (units[Zerg_Overlord] < 1))
        return false;
    
    BWTA::BaseLocation* home = BWTA::getStartLocation(self);
    if (home == NULL)
        return false;
    
    if (home->getGeysers().size() < 1)
        return false;
    
    return true;
}

#define multi(num) for(int k=0; k<num; ++k)

void ZergStrategieCode::onMatchBegin()
{
    zergling_gt_level = 0;

    initStandardPlaning(Races::Zerg);
    setGasOnDemand(true);
    setIndexCoeff(1000);

    useScout(morphUnit(Zerg_Drone));
    switch (rand() % 4)
    {
        case 0:
            LOG << "Opening with 4 pool...";
            buildUnitEx(Zerg_Spawning_Pool);
            multi(6) morphAttackUnit(Zerg_Zergling);
            break;

        case 1:
            LOG << "Opening with 5 pool...";
            morphWorker(Zerg_Drone);
            buildUnitEx(Zerg_Spawning_Pool);
            multi(2) morphWorker(Zerg_Drone);
            multi(3) morphAttackUnit(Zerg_Zergling);
            morphUnitEx(Zerg_Overlord);
            multi(3) morphAttackUnit(Zerg_Zergling);
            break;
        
        case 2:
            LOG << "Opening with overpool...";
            multi(5) morphWorker(Zerg_Drone);
            morphUnitEx(Zerg_Overlord);
            buildUnitEx(Zerg_Spawning_Pool);
            multi(2) morphWorker(Zerg_Drone);
            multi(6) morphAttackUnit(Zerg_Zergling);
            break;

        case 3:
            LOG << "Opening with 12hatch...";
            multi(5) morphWorker(Zerg_Drone);
            morphUnitEx(Zerg_Overlord);
            multi(3) morphWorker(Zerg_Drone);
            buildUnitEx(Zerg_Hatchery);
            morphWorker(Zerg_Drone);
            buildUnitEx(Zerg_Spawning_Pool);
            buildRefinery(Zerg_Extractor);
            multi(2) morphWorker(Zerg_Drone);
            //upgradeTechEx(nextZerglingUpgrade());
            multi(10) morphAttackUnit(Zerg_Zergling);
            break;
    }
}

void ZergStrategieCode::onTick()
{
    int time = Broodwar->getFrameCount();
    if (time % 10 != 8)
        return;

    bool needsZerglings = true;
    bool needsWorker    = freeMineralLinePlaces() > 0;
    bool needsHatchery  = buildUnitPlanSize(Zerg_Hatchery) == 0;
    bool needsTech      = numberOfPlanedTechs() == 0;

    bool larvaAvailable = nextFreeLarvaTime() < time + timehorizon;
    bool resAvailable   = estimateResourcesAt(time + timehorizon) > 100;

    if (larvaAvailable && resAvailable) {
        if (needsWorker && (rand() % 4 == 0)) {
            morphWorker(Zerg_Drone);
        } else if (needsZerglings) {
            morphAttackUnit(Zerg_Zergling);
        }
    }
    
    if (resAvailable && !larvaAvailable) {
        if (needsTech && (rand() % 2 == 0)) {
            // Build refinery, if not jet done.
            if (nextUnitAvailable(Zerg_Extractor) > Precondition::Max)
                buildRefinery(Zerg_Extractor);
            UpgradeType gt = nextZerglingUpgrade();
            // Build evolution chamber, if needed and not jet done.
            if (gt == Zerg_Carapace || gt == Zerg_Melee_Attacks)
                if (nextUnitAvailable(Zerg_Evolution_Chamber) > Precondition::Max)
                    buildUnitEx(Zerg_Evolution_Chamber);
            if (gt != UpgradeTypes::Unknown) {
                LOG << "Upgrading '" << gt << "'.";
                upgradeTechEx(gt);
            }
        } else if (needsHatchery) {
            if (rand() % 2 == 0) {
                LOG << "Building new hatchery within base.";
                buildUnitEx(Zerg_Hatchery);
            } else {
                LOG << "Building new expo.";
                buildExpo();
            }
        }
    }
}

void ZergStrategieCode::onMatchEnd()
{ }

namespace
{
    bool showValues = false;
}

void ZergStrategieCode::onSendText(const std::string& text)
{
    if (text == "/show values")
        showValues = !showValues;
}

void ZergStrategieCode::onDrawPlan(HUDTextOutput& hud)
{
    if (!showValues) return;

    int time = Broodwar->getFrameCount();

    hud.printf(" ");
    hud.printf("Zerg decision values:");
    hud.printf(" - free mineral places: %d", freeMineralLinePlaces());
    hud.printf(" - next free larva time: %d", nextFreeLarvaTime());
    hud.printf(" - number of planed techs: %d", numberOfPlanedTechs());
    hud.printf(" - resources in +%d: %d", timehorizon, estimateResourcesAt(time + timehorizon));
}

void ZergStrategieCode::onBaseMinedOut(BWTA::BaseLocation* /*base*/)
{
    LOG << "Building new expo, because other is mined out.";
    buildExpo();
}
