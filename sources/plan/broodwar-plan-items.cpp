#include "broodwar-plan-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-actions.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"
#include "engine/broodwar-events.hpp"

OwnUnitPlanItem::OwnUnitPlanItem(BWAPI::Unit* u)
    : AbstractBoundaryItem(u), provideUnit(u)
{
    ports.push_back(&provideUnit);
    provideUnit.estimatedTime = ACTIVE_TIME;
}

void OwnUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnUnitPlanItem(this);
}

void OwnUnitPlanItem::visitUnitUpdateEvent(UnitUpdateEvent* event)
{
    provideUnit.updateData(event->unitType, event->pos);
}

AbstractSimpleUnitPlanItem::AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od)
    : requireUnit(ut), provideUnit(NULL, od)
{
    ports.push_back(&requireUnit);
    ports.push_back(&provideUnit);
}

void AbstractSimpleUnitPlanItem::updateEstimates()
{
    requireUnit.updateEstimates();
    AbstractPlanItem::updateEstimates();
}

AbstractAction* AbstractSimpleUnitPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    BWAPI::Unit* unit = requireUnit.getUnit();
    provideUnit.setUnit(unit);
    return requireUnit.prepareForExecution(engine);
}

void AbstractSimpleUnitPlanItem::removeFinished(AbstractAction* /*action*/)
{
    requireUnit.bridge(&provideUnit);
}

GatherMineralsPlanItem::GatherMineralsPlanItem(BWAPI::Unit* m, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(BWAPI::UnitTypes::Zerg_Drone, true), mineral(m)
{
    provideUnit.updateData(BWAPI::UnitTypes::Zerg_Drone, BWAPI::Positions::Unknown);
    if (provider != NULL)
        requireUnit.connectTo(provider);
}

void GatherMineralsPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitGatherMineralPlanItem(this);
}

void GatherMineralsPlanItem::updateEstimates()
{
    AbstractSimpleUnitPlanItem::updateEstimates();

    int newvalue = estimatedStartTime;
    if (newvalue != provideUnit.estimatedTime) {
        provideUnit.estimatedTime = newvalue;
        //LOG << "Update endTime to " << newvalue;
    }
}

AbstractAction* GatherMineralsPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    //LOG << "Prepare for execution(GatherMinerals) ...";
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new CollectMineralsAction(requireUnit.getUnit(), mineral, req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}

BuildPlanItem::BuildPlanItem(BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractSimpleUnitPlanItem(ut.whatBuilds().first), resources(ut.mineralPrice(), ut.gasPrice()), unitType(ut), pos(p)
{
    ports.push_back(&resources);
    provideUnit.updateData(unitType, BWAPI::Position(pos));
}

void BuildPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitBuildPlanItem(this);
}

void BuildPlanItem::updateEstimates()
{
    AbstractSimpleUnitPlanItem::updateEstimates();
    provideUnit.estimatedTime = estimatedStartTime + unitType.buildTime();
}

AbstractAction* BuildPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    //LOG << "Prepare for execution(Build) ...";
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new ZergBuildAction(requireUnit.getUnit(), unitType, pos, req);
    engine->addAction(action);
    provideUnit.setPreviousAction(action);
    return action;
}
