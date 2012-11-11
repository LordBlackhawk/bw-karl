#include "scout.hpp"
#include "bwapi-precondition.hpp"
#include "container-helper.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include "random-chooser.hpp"

#include <BWTA.h>
#include <vector>
#include <algorithm>

using namespace BWAPI;
using namespace BWTA;

#define THIS_DEBUG DEBUG

namespace
{
    struct ScoutInformation
    {
        Unit* scout;
        // more informations!!!
        
        ScoutInformation(Unit* s)
            : scout(s)
        { }
        
        Position getNextPosition()
        {
            if (rand() % 2 == 0) {
                return getRandomSomething(BWTA::getChokepoints())->getCenter();
            } else {
                return getRandomSomething(BWTA::getBaseLocations())->getPosition();
            }
        }
        
        void sendScout()
        {
            THIS_DEBUG << "Sending scout!";
            scout->rightClick(getNextPosition());
        }
        
        bool onTick()
        {
            if (!scout->isMoving())
                sendScout();

            return !scout->exists();
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