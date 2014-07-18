#include "broodwar-actions.hpp"
#include "utils/log.hpp"
#include <cstring>

UnitAction::UnitAction(BWAPI::Unit* u, AbstractAction* pre)
    : AbstractAction(pre), unit(u)
{ }

void UnitAction::drawInformations(const char* name)
{
    BWAPI::Position pos = unit->getPosition();
    BWAPI::Broodwar->drawTextMap(pos.x() - 2*strlen(name), pos.y(), "%s", name);
}

CollectMineralsAction::CollectMineralsAction(BWAPI::Unit* w, BWAPI::Unit* m, AbstractAction* pre)
    : UnitAction(w, pre), mineral(m)
{ }

CollectMineralsAction::Status CollectMineralsAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    if (!mineral->exists())
        return Finished;

    if (!unit->isGatheringMinerals())
        unit->rightClick(mineral);

    drawInformations("gathering minerals");
    return Running;
}

ZergBuildAction::ZergBuildAction(BWAPI::Unit* w, BWAPI::UnitType ut, BWAPI::TilePosition p, AbstractAction* pre)
    : UnitAction(w, pre), unitType(ut), pos(p)
{ }

void ZergBuildAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    //LOG << "ZergBuildAction begin...";
    unit->stop();
}

ZergBuildAction::Status ZergBuildAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    BWAPI::UnitType type = unit->getType();
    if (type == unitType) {
        drawInformations("building");
        return unit->isCompleted() ? Finished : Running;
    }

    drawInformations("prepare building");
    if (type != BWAPI::UnitTypes::Zerg_Drone)
        return Failed;

    if (!unit->isIdle() && !unit->isGatheringMinerals())
        return Running;

    if (unit->build(pos, unitType))
        return Running;

    BWAPI::Error err = BWAPI::Broodwar->getLastError();
    if (err == BWAPI::Errors::Insufficient_Minerals)
        return Running;
    if (err == BWAPI::Errors::Insufficient_Gas)
        return Running;
    if (err == BWAPI::Errors::Unit_Busy)
        return Running;
    /*
    if (err == BWAPI::Errors::Unreachable_Location)
        return Failed;
    if (err == BWAPI::Errors::Unbuildable_Location)
        return Failed;
    if (err == BWAPI::Errors::Insufficient_Tech)
        return Failed;
    */
    LOG << "ZergBuildAction failed with " << err.toString();
    return Failed;
}

void ZergBuildAction::onEnd(AbstractExecutionEngine* /*engine*/)
{
    //LOG << "ZergBuildAction finished.";
    // If the unit still exists and the building is not yet finished, abort.
    if (unit->exists()) {
        if (unit->isBeingConstructed()) {
            unit->cancelConstruction();
        } else {
            unit->stop();
        }
    }
}

MoveToPositionAction::MoveToPositionAction(BWAPI::Unit* w, BWAPI::Position p, AbstractAction* pre)
    : UnitAction(w, pre), pos(p)
{ }

MoveToPositionAction::Status MoveToPositionAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    if (unit->getPosition().getDistance(pos) < 32.0)
        return Finished;

    if (!unit->isMoving())
        unit->move(pos);

    drawInformations("moving");
    return Running;
}

MineralTrigger::MineralTrigger(int a, AbstractAction* pre)
    : AbstractAction(pre), amount(a)
{ }

MineralTrigger::Status MineralTrigger::onTick(AbstractExecutionEngine* /*engine*/)
{
    return (BWAPI::Broodwar->self()->minerals() >= amount) ? Finished : Running;
}



SendTextAction::SendTextAction(std::string msg, bool toAlliesOnly, AbstractAction* pre)
    : AbstractAction(pre), message(msg), toAllies(toAlliesOnly)
{
    timeout=24*10+message.length()*8; //wait some ticks, TODO: make this configurable? Wait for a certain Time instead of Ticks?
}

void SendTextAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    BWAPI::Broodwar->printf("sending: '%s' to %s",message.c_str(),toAllies?"allies only":"everyone");
    BWAPI::Broodwar->sendTextEx(toAllies,"%s",message.c_str());    
}

SendTextAction::Status SendTextAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if(--timeout>0)
        return Running;
    return Finished;
}

GiveUpAction::GiveUpAction(AbstractAction* pre)
    : SendTextAction("GG",false,pre)
{ }

GiveUpAction::Status GiveUpAction::onTick(AbstractExecutionEngine* engine)
{
    switch(SendTextAction::onTick(engine))
    {
        case Running:
            return Running;
        case Failed:
            return Failed;
        default:
            BWAPI::Broodwar->leaveGame();
            return Finished;
    }
}
