#include "broodwar-plan-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-actions.hpp"
#include "utils/log.hpp"

#include <cassert>

OwnUnitPlanItem::OwnUnitPlanItem(BWAPI::Unit* u)
    : provideUnit(u)
{
    ports.push_back(&provideUnit);
    estimatedStartTime = ACTIVE_TIME;
    provideUnit.estimatedTime = ACTIVE_TIME;
}

void OwnUnitPlanItem::updateData(BWAPI::UnitType ut, BWAPI::Position p)
{
    provideUnit.updateData(ut, p);
}

void OwnUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnUnitPlanItem(this);
}

bool OwnUnitPlanItem::prepareForExecution(AbstractExecutionEngine* /*engine*/)
{
    assert(false && "OwnUnitPlanItem should not be called for prepareForExecution.");
}

GatherMineralsPlanItem::GatherMineralsPlanItem(BWAPI::Unit* m, ProvideUnitPort* provider)
    : requireUnit(BWAPI::UnitTypes::Zerg_Drone), provideUnit(NULL, true), mineral(m)
{
    ports.push_back(&requireUnit);
    ports.push_back(&provideUnit);
    if (provider != NULL)
        requireUnit.connectTo(provider);
}

void GatherMineralsPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitGatherMineralPlanItem(this);
}

void GatherMineralsPlanItem::updateEstimates()
{
    requireUnit.updateEstimates();
    AbstractPlanItem::updateEstimates();

    int newvalue = estimatedStartTime;
    if (newvalue != provideUnit.estimatedTime) {
        provideUnit.estimatedTime = newvalue;
        LOG << "Update endTime to " << newvalue;
    }
}

bool GatherMineralsPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    provideUnit.updateData(&requireUnit);
    AbstractAction* action = new CollectMineralsAction(requireUnit.getUnit(), mineral, requireUnit.prepareForExecution(engine));
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return true;
}

BuildPlanItem::BuildPlanItem(BWAPI::UnitType ut, BWAPI::TilePosition p)
    : requireUnit(ut.whatBuilds().first), provideUnit(NULL), unitType(ut), pos(p)
{
    ports.push_back(&requireUnit);
    ports.push_back(&provideUnit);
    provideUnit.updateData(unitType, BWAPI::Position(pos));
}

void BuildPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitBuildPlanItem(this);
}

void BuildPlanItem::updateEstimates()
{
    requireUnit.updateEstimates();
    AbstractPlanItem::updateEstimates();
    provideUnit.estimatedTime = estimatedStartTime + unitType.buildTime();
}

bool BuildPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    BWAPI::Unit* unit = requireUnit.getUnit();
    provideUnit.setUnit(unit);
    AbstractAction* action = new ZergBuildAction(unit, unitType, pos, requireUnit.prepareForExecution(engine));
    engine->addAction(action);
    provideUnit.setPreviousAction(action);
    return true;
}
