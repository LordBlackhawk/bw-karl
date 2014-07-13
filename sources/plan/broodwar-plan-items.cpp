#include "broodwar-plan-items.hpp"
#include "broodwar-boundary-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-actions.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"

AbstractSimpleUnitPlanItem::AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od)
    : requireUnit(this, ut), provideUnit(this, NULL, od)
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

GatherMineralsPlanItem::GatherMineralsPlanItem(MineralBoundaryItem* m, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(BWAPI::UnitTypes::Zerg_Drone, true), requireMineralField(this, m)
{
    ports.push_back(&requireMineralField);
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
    requireMineralField.updateEstimates();
    AbstractSimpleUnitPlanItem::updateEstimates();
    provideUnit.estimatedTime = estimatedStartTime;
    //LOG << "estimated start time: " << estimatedStartTime << "; requireMineralField: " << requireMineralField.estimatedTime << "; requireUnit: " << requireUnit.estimatedTime;
}

AbstractAction* GatherMineralsPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    //LOG << "Prepare for execution(GatherMinerals) ...";
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new CollectMineralsAction(requireUnit.getUnit(), requireMineralField.getUnit(), req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}

BuildPlanItem::BuildPlanItem(Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractSimpleUnitPlanItem(ut.whatBuilds().first),
      requireResources(this, ut.mineralPrice(), ut.gasPrice()),
      requireSpace(this, f, ut.tileWidth(), ut.tileHeight(), p),
      unitType(ut)
{
    ports.push_back(&requireResources);
    provideUnit.updateData(unitType, BWAPI::Position(p));
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
    AbstractAction* action = new ZergBuildAction(requireUnit.getUnit(), unitType, requireSpace.getTilePosition(), req);
    engine->addAction(action);
    provideUnit.setPreviousAction(action);
    return action;
}
