#include "broodwar-actions.hpp"
#include "broodwar-events.hpp"
#include "utils/log.hpp"
#include "utils/bw-helper.hpp"
#include "utils/options.hpp"
#include <cstring>
#include <math.h>   // muss ich das hier machen??

#define M_PI 3.141592653589793238462643383279502884L

UnitAction::UnitAction(BWAPI::Unit* u, AbstractAction* pre)
    : AbstractAction(pre), unit(u)
{ }

void UnitAction::drawInformations(const char* name)
{
    if (!OptionsRegistrar::optHUD())
        return;
    BWAPI::Position pos = unit->getPosition();
    BWAPI::Broodwar->drawTextMap(pos.x() - 2*strlen(name), pos.y(), "%s", name);
}


CollectResourcesAction::CollectResourcesAction(BWAPI::Unit* w, BWAPI::Unit* m, AbstractAction* pre)
    : UnitAction(w, pre), resource(m)
{ }

CollectResourcesAction::Status CollectResourcesAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    if (!resource->exists())
        return Finished;

    if (unit->isGatheringMinerals())
        drawInformations("gathering minerals");
    else if (unit->isGatheringGas())
        drawInformations("gathering gas");
    else
        unit->rightClick(resource);


    return Running;
}


ZergBuildAction::ZergBuildAction(BWAPI::Unit* w, BWAPI::UnitType ut, BWAPI::TilePosition p, AbstractAction* pre)
    : UnitAction(w, pre), unitType(ut), pos(p), resourcesConsumed(false)
{ }

void ZergBuildAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    unit->stop();
    resourcesConsumed = false;
}

ZergBuildAction::Status ZergBuildAction::onTick(AbstractExecutionEngine* engine)
{
    if (!unit->exists()) {
        if (unitType == BWAPI::UnitTypes::Zerg_Extractor) {
            BWAPI::Unit* newUnit = NULL;
            for (auto it : BWAPI::Broodwar->self()->getUnits())
                if ((it->getType() == unitType) && (it->getTilePosition() == pos))
                    newUnit = it;
            if (newUnit == NULL)
                return Failed;
            unit = newUnit;
            return Running;
        }
        return Failed;
    }

    BWAPI::UnitType type = unit->getType();
    if (type == unitType) {
        drawInformations("building");
        if (!resourcesConsumed) {
            engine->generateEvent(new ResourcesConsumedEvent(this));
            resourcesConsumed = true;
        }
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
    LOG << "ZergBuildAction(" << unitType << ") failed with " << err;
    return Failed;
}

void ZergBuildAction::onEnd(AbstractExecutionEngine* /*engine*/)
{
    // If the unit still exists and the building is not yet finished, abort.
    if (unit->exists()) {
        if (unit->isBeingConstructed()) {
            unit->cancelConstruction();
        } else {
            unit->stop();
        }
    }
}


MorphUnitAction::MorphUnitAction(BWAPI::Unit* u, BWAPI::UnitType to, AbstractAction* pre)
    : UnitAction(u, pre), unitType(to), resourcesConsumed(false)
{ }

void MorphUnitAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    resourcesConsumed = false;
}

MorphUnitAction::Status MorphUnitAction::onTick(AbstractExecutionEngine* engine)
{
    if(!unit->exists())
        return Failed;

    if(unit->isMorphing())
    {
        drawInformations("morphing");
        if (!resourcesConsumed) {
            engine->generateEvent(new ResourcesConsumedEvent(this));
            resourcesConsumed = true;
        }
        return Running;
    }

    if(unit->getType() == unitType)
        return Finished;

    if (unit->morph(unitType))
        return Running;

    BWAPI::Error err = BWAPI::Broodwar->getLastError();
    if (err == BWAPI::Errors::Insufficient_Minerals)
        return Running;
    if (err == BWAPI::Errors::Insufficient_Gas)
        return Running;
    if (err == BWAPI::Errors::Unit_Busy || err == BWAPI::Errors::Incompatible_State)
    {
        //LOG << "MorphUnitAction: unit busy - trying to stop: " << err.toString();
        unit->stop();
        return Running;
    }
    LOG << "MorphUnitAction failed with " << err;
    return Failed;
}

void MorphUnitAction::onEnd(AbstractExecutionEngine* /*engine*/)
{
    /*
    LOG << "MorphUnitAction finished.";
    // If the unit still exists and the morphing is not yet finished, abort.
    if (unit->exists())
    {
        if (unit->isMorphing())
        {
            unit->cancelMorph();
        }
        else
        {
            unit->stop();
        }
    }*/
}


MoveToPositionAction::MoveToPositionAction(BWAPI::Unit* w, BWAPI::Position p, bool useSTA, AbstractAction* pre)
    : UnitAction(w, pre), pos(p), useSmartTurnAround(useSTA)
{ }

void MoveToPositionAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    unit->move(pos);
    isTurning = useSmartTurnAround;
}

MoveToPositionAction::Status MoveToPositionAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    BWAPI::Position myPos = unit->getPosition();
    double dist = myPos.getDistance(pos);
    if (dist < 64.0)
        {
        unit->move(pos);   // if we want to move for less than 64 in micro management
        return Finished;
        }

    if(!isTurning) drawInformations("moving");
    if (OptionsRegistrar::optHUD())
        BWAPI::Broodwar->drawLineMap(myPos.x(), myPos.y(), pos.x(), pos.y(), BWAPI::Colors::Green);
    if (unit->isMoving())
        {
        // optimizing the turning around of units
        if(isTurning) //&& (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord))
            {
            drawInformations("turning");
            double cosAngle = (pos.x() - myPos.x())/dist;
            double sinAngle = (pos.y() - myPos.y())/dist;
            if(firstCall == true) 
				{
					currentAngle = unit->getAngle();
					firstCall = false;
				}
            if ((std::abs(cos(currentAngle) - cosAngle) + std::abs(sin(currentAngle) - sinAngle)) > 0.1)
                {
                    double angle = atan2(sinAngle, cosAngle);
                    if (angle < 0) angle = angle + 2*M_PI;
                    double diffAngle = angle-currentAngle;
                    // Rechts oder Links herum drehen?
                    if(diffAngle > M_PI) diffAngle = diffAngle-2*M_PI;
                    if(diffAngle < -M_PI) diffAngle = 2*M_PI+diffAngle;
					// Sign + cutoff function
                    if(diffAngle > 0.2) diffAngle = 0.2;
					if(diffAngle < -0.2) diffAngle = -0.2;
					currentAngle = currentAngle + diffAngle;
                    unit->move(BWAPI::Position(myPos.x()+round(100*cos(currentAngle)),myPos.y()+round(100*sin(currentAngle))));
					//BWAPI::Broodwar->drawLineMap(myPos.x(), myPos.y(), myPos.x()+round(100*cos(currentAngle)), myPos.y()+round(100*sin(currentAngle)), BWAPI::Colors::Blue);
                }
            else
                {
                    unit->move(pos);
                    isTurning = false;
                }
			double vx = unit->getVelocityX();
			double vy = unit->getVelocityY();
			double vmax = unit->getType().topSpeed();
			if((vx*vx+vy*vy)/(vmax*vmax) < 0.1) 
				{
					unit->move(pos);
                    isTurning = false;
				}
            }
        return Running;
        }

    if (unit->move(pos))
        return Running;

    LOG << "MoveToPositionAction failed with " << BWAPI::Broodwar->getLastError();
    return Failed;
}


AttackPositionAction::AttackPositionAction(BWAPI::Unit* w, BWAPI::Position p, AbstractAction* pre)
    : UnitAction(w, pre), pos(p)
{ }

void AttackPositionAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    unit->attack(pos);
}

AttackPositionAction::Status AttackPositionAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    BWAPI::Position myPos = unit->getPosition();
    if (myPos.getDistance(pos) < 32.0)
		{
			unit->attack(pos);
			return Finished;
		}

    drawInformations("attackingPosition");
    if (OptionsRegistrar::optHUD())
        BWAPI::Broodwar->drawLineMap(myPos.x(), myPos.y(), pos.x(), pos.y(), BWAPI::Colors::Green);
    if(!unit->isIdle())
        return Running;

    if (unit->attack(pos))
        return Running;

    LOG << "AttackPositionAction failed with " << BWAPI::Broodwar->getLastError();
    return Failed;
}


AttackUnitAction::AttackUnitAction(BWAPI::Unit* w, BWAPI::Unit* enemy, AbstractAction* pre)
    : UnitAction(w, pre), e(enemy)
{ }

void AttackUnitAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    unit->attack(e);
}

AttackUnitAction::Status AttackUnitAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    if (!e->exists())
        return Finished;

    drawInformations("attackingUnit");
    if (OptionsRegistrar::optHUD()) {
        BWAPI::Position myPos = unit->getPosition();
        BWAPI::Position pos = e->getPosition();
        BWAPI::Broodwar->drawLineMap(myPos.x(), myPos.y(), pos.x(), pos.y(), BWAPI::Colors::Green);
    }
    if (!unit->isIdle())
        return Running;

    if (unit->attack(e))
        return Running;

    LOG << "AttackUnitAction failed with " << BWAPI::Broodwar->getLastError();
    return Failed;
}

ResearchTechAction::ResearchTechAction(BWAPI::Unit* myunit, BWAPI::TechType t, AbstractAction* pre)
    : UnitAction(myunit, pre), tech(t), waitFrames(5)
{ }

ResearchTechAction::Status ResearchTechAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    if (!unit->exists())
        return Failed;

    if (unit->isResearching()) {
        waitFrames = -1;
        return Running;
    }

    if (!unit->isIdle())
        return Running;

    if (waitFrames == -1)
        return Finished;

    if (waitFrames > 0) {
        --waitFrames;
        return Running;
    }

    if (unit->research(tech))
        return Running;

    LOG << "ResearchTechAction failed with " << BWAPI::Broodwar->getLastError();
    return Failed;
}


UpgradeAction::UpgradeAction(BWAPI::Unit* myunit, BWAPI::UpgradeType u, AbstractAction* pre)
    : UnitAction(myunit, pre), upgrade(u), waitFrames(5)
{ }

UpgradeAction::Status UpgradeAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    // There is a bug in StarCraft: If upgrade(...) is called one of the first frames after a building is finished, the upgrade does not really start but blocks the building.

    if (!unit->exists())
        return Failed;

    if (unit->isUpgrading()) {
        waitFrames = -1;
        return Running;
    }

    if (!unit->isIdle())
        return Running;

    if (waitFrames == -1)
        return Finished;

    if (waitFrames > 0) {
        --waitFrames;
        return Running;
    }

    if (unit->upgrade(upgrade))
        return Running;

    LOG << "UpgradeAction failed with " << BWAPI::Broodwar->getLastError();
    return Failed;
}


SendTextAction::SendTextAction(std::string msg, bool toAlliesOnly, AbstractAction* pre)
    : AbstractAction(pre), message(msg), toAllies(toAlliesOnly)
{
    timeout=24*10+message.length()*8; //wait some ticks, TODO: make this configurable? Wait for a certain Time instead of Ticks?
}

void SendTextAction::onBegin(AbstractExecutionEngine* /*engine*/)
{
    LOG << "GG.";
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
