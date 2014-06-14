#include "micro-movement.hpp"
#include "log.hpp"
#include "random-chooser.hpp"

using namespace BWAPI;
using namespace BWTA;

MicroMovement::MicroMovement(BWAPI::Unit* u)
    : unit(u), nextCommandFrame(0), nextMacroFrame(0), state(sIdle), finishState(sIdle)
{ }

MicroMovement::~MicroMovement()
{ }

bool MicroMovement::onTick()
{
    if (!unit->exists())
        return true;
    
    int currentFrame = Broodwar->getFrameCount();
    if (currentFrame < nextCommandFrame)
        return false;

    if (nextMacroFrame <= currentFrame) {
        onMacroTick();
        nextMacroFrame = currentFrame + 10;
    }

    switch (state)
    {
        case sIdle:
            nextCommandFrame = nextMacroFrame;
            break;

        case sMoving:
            if (unit->getPosition().getDistance(targetPos) < 80.0) {
                state = finishState;
                break;
            }
            if (!unit->isIdle()) {
                nextCommandFrame = currentFrame + 5;
                break;
            }
        case sCommandMove:
            unit->rightClick(targetPos);
            state = sMoving;
            nextCommandFrame = currentFrame + 5;
            break;

        case sScoutRegion:
            moveTo(getRandomSomething(targetRegion->getPolygon().points));
            finishState = sScoutRegion;
            break;

        default:
            WARNING << "Unknown MicroMovement::InternalState";
            break;
    }

    return false;
}

void MicroMovement::onMacroTick()
{ }

bool MicroMovement::isIdle()
{
    return (state == sIdle);
}

BWTA::Region* MicroMovement::currentRegion()
{
    return getRegion(unit->getPosition());
}

void MicroMovement::moveTo(const BWAPI::Position& target)
{
    targetPos       = target;
    state           = sCommandMove;
    finishState     = sIdle;
}

void MicroMovement::scoutRegion(BWTA::Region* region)
{
    targetRegion    = region;
    state           = sScoutRegion;
    finishState     = sIdle;
}
