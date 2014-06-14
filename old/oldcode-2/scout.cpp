#include "scout.hpp"
#include "container-helper.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include "random-chooser.hpp"
#include "micro-movement.hpp"
#include "information-collector.hpp"

#include <BWTA.h>
#include <vector>
#include <algorithm>

using namespace BWAPI;
using namespace BWTA;

#define THIS_DEBUG DEBUG

namespace
{
    struct ScoutInformation : public MicroMovement
    {
        ScoutInformation(Unit* s)
            : MicroMovement(s)
        { }

        Position getNextPosition()
        {
            std::set<BaseLocation*> locs = getUnscoutedStartLocations();
            if (locs.empty())
                locs = getUnscoutedBaseLocations();
            if (locs.empty())
                locs = BWTA::getBaseLocations();
            if (locs.empty())
                return Positions::Unknown;
            return getRandomSomething(locs)->getPosition();
        }

        void onMacroTick()
        {
            if (!isIdle())
                return;
            BaseLocation* enemyBase = getEnemyStartLocation();
            if (enemyBase == NULL) {
                THIS_DEBUG << "Sending scout!";
                moveTo(getNextPosition());
                return;
            }
            THIS_DEBUG << "enemy base found, scouting region!";
            scoutRegion(currentRegion());
        }
    };

    std::vector<UnitPrecondition*>  assigned_scouts;
    std::vector<ScoutInformation*>  scouts;

    bool isScoutReady(UnitPrecondition* pre)
    {
        if (pre->isFulfilled()) {
            THIS_DEBUG << "Scout finished!";
            scouts.push_back(new ScoutInformation(pre->unit));
            release(pre);
            return true;
        }
        return false;
    }
}

void useScout(UnitPrecondition* pre)
{
    if (pre == NULL)
        return;

    THIS_DEBUG << "Scout assigned!";
    assigned_scouts.push_back(pre);
}

void ScoutCode::onMatchBegin()
{

}

void ScoutCode::onMatchEnd()
{
    Containers::clear_and_delete(assigned_scouts);
    Containers::clear_and_delete(scouts);
}

void ScoutCode::onTick()
{
    Containers::remove_if(assigned_scouts, isScoutReady);
    Containers::remove_if_delete(scouts, std::mem_fun(&ScoutInformation::onTick));
}