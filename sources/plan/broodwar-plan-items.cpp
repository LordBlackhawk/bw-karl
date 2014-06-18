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
    : requireUnit(BWAPI::UnitTypes::Zerg_Drone), provideUnit(NULL), mineral(m)
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

    int newvalue = estimatedStartTime + INFINITE_TIME;
    if (newvalue != provideUnit.estimatedTime) {
        provideUnit.estimatedTime = newvalue;
        LOG << "Update endTime to " << newvalue;
    }
}

bool GatherMineralsPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    provideUnit.updateData(&requireUnit);
    engine->addAction(new CollectMineralsAction(requireUnit.getUnit(), mineral, NULL));
    return true;
}
