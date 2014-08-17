#include "broodwar-plan-items.hpp"
#include "broodwar-boundary-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-actions.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"

AbstractSimpleUnitPlanItem::AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od)
    : requireUnit(this, ut), provideUnit(this, NULL, od)
{ }

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


GatherMineralsPlanItem::GatherMineralsPlanItem(ResourceBoundaryItem* m, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(provider->getUnitType(), true), requireMineralField(this, m)
{
    provideUnit.updateData(provider->getUnitType(), BWAPI::Position(m->getTilePosition()));
    if (provider != NULL)
        requireUnit.connectTo(provider);
}

void GatherMineralsPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitGatherMineralPlanItem(this);
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


MorphUnitPlanItem::MorphUnitPlanItem(BWAPI::UnitType type, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(type.whatBuilds().first),
      requireResources(this, type.mineralPrice(), type.gasPrice()),
      supply(this, type, true),
      unitType(type)
{
    provideUnit.updateData(unitType, (provider != NULL) ? provider->getPosition() : BWAPI::Positions::Unknown);
    supply.estimatedDuration = provideUnit.estimatedDuration = unitType.buildTime();
    requireUnit.connectTo(provider);
}

void MorphUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitMorphUnitPlanItem(this);
}

AbstractAction* MorphUnitPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new MorphUnitAction(requireUnit.getUnit(), unitType, req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}

void MorphUnitPlanItem::visitResourcesConsumedEvent(ResourcesConsumedEvent* /*event*/)
{
    setExecuting();
    removePort(&requireResources);
    if (supply.isRequirePort())
        removePort(&supply);
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

void MoveToPositionPlanItem::updateEstimates(Time current)
{
    provideUnit.estimatedDuration = (int)(position.getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
    AbstractSimpleUnitPlanItem::updateEstimates(current);
}

AbstractAction* MoveToPositionPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new MoveToPositionAction(requireUnit.getUnit(), position, req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}



AttackUnitPlanItem::AttackUnitPlanItem(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy)
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), enemyUnit(this,enemy)
{
    provideUnit.updateData(provider->getUnitType(), enemy->getPosition());
    requireUnit.connectTo(provider);
}

void AttackUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitAttackUnitPlanItem(this);
}

void AttackUnitPlanItem::updateEstimates(Time current)
{
    provideUnit.estimatedDuration = (int)(enemyUnit.getPosition().getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
    AbstractSimpleUnitPlanItem::updateEstimates(current);
}

AbstractAction* AttackUnitPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* req = AbstractSimpleUnitPlanItem::prepareForExecution(engine);
    AbstractAction* action = new AttackUnitAction(requireUnit.getUnit(), enemyUnit.getUnit(), req);
    provideUnit.setPreviousAction(action);
    engine->addAction(action);
    return action;
}




BuildPlanItem::BuildPlanItem(Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractSimpleUnitPlanItem(ut.whatBuilds().first),
      requireResources(this, ut.mineralPrice(), ut.gasPrice()),
      requireSpace(this, f, ut, p),
      supply(this, ut),
      unitType(ut)
{
    provideUnit.updateData(unitType, BWAPI::Position(p));
    supply.estimatedDuration = provideUnit.estimatedDuration = unitType.buildTime();
}

void BuildPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitBuildPlanItem(this);
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
    setExecuting();
    removePort(&requireResources);
    removePort(&requireSpace);
}
