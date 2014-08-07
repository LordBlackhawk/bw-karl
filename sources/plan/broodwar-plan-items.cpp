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

// TODO: Should be provider->getUnitType(), BWAPI::Position(m->getTilePosition())!!!!
GatherMineralsPlanItem::GatherMineralsPlanItem(ResourceBoundaryItem* m, ProvideUnitPort* provider)
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

MoveToPositionPlanItem::MoveToPositionPlanItem(ProvideUnitPort* provider, BWAPI::Position p)
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), position(p)
{
    provideUnit.updateData(provider->getUnitType(), position);
    requireUnit.connectTo(provider);
}

void MoveToPositionPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitMoveToPositionPlanItem(this);
}

void MoveToPositionPlanItem::updateEstimates()
{
    AbstractSimpleUnitPlanItem::updateEstimates();
    provideUnit.estimatedTime = estimatedStartTime + (int)(position.getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
}

AbstractAction* MoveToPositionPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new MoveToPositionAction(requireUnit.getUnit(), position, req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}



AttackUnitPlanItem::AttackUnitPlanItem(ProvideUnitPort* provider, ProvideUnitPort* enemy)
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), enemyunit(enemy)
{
    provideUnit.updateData(provider->getUnitType(), enemy);
    requireUnit.connectTo(provider);
}

void AttackUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitAttackUnitPlanItem(this);
}

void AttackUnitPlanItem::updateEstimates()
{
    AbstractSimpleUnitPlanItem::updateEstimates();
    provideUnit.estimatedTime = estimatedStartTime + (int)(enemyunit.getPosition().getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
}

AbstractAction* AttackUnitPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new AttackUnitAction(requireUnit.getUnit(), requireUnit.getUnit(), req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}




BuildPlanItem::BuildPlanItem(Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractSimpleUnitPlanItem(ut.whatBuilds().first),
      requireResources(this, ut.mineralPrice(), ut.gasPrice()),
      requireSpace(this, f, ut, p),
      unitType(ut)
{
    ports.push_back(&requireResources);
    ports.push_back(&requireSpace);
    provideUnit.updateData(unitType, BWAPI::Position(p));
}

void BuildPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitBuildPlanItem(this);
}

void BuildPlanItem::updateEstimates()
{
    requireSpace.updateEstimates();
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

void BuildPlanItem::visitResourcesConsumedEvent(ResourcesConsumedEvent* /*event*/)
{
    removePort(&requireResources);
    removePort(&requireSpace);
}
