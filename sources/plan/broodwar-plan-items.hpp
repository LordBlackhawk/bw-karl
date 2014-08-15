#pragma once

#include "broodwar-ports.hpp"

class ResourceBoundaryItem;

class AbstractSimpleUnitPlanItem : public AbstractPlanItem
{
    public:
        RequireUnitPort     requireUnit;
        ProvideUnitPort     provideUnit;

        AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od = false);

        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
        void removeFinished(AbstractAction* action) override;
};

class GatherMineralsPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        RequireMineralFieldPort requireMineralField;

        GatherMineralsPlanItem(ResourceBoundaryItem* m, ProvideUnitPort* provider);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
};

class MorphUnitPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort requireResources;
        SupplyPort supply;

        MorphUnitPlanItem(BWAPI::UnitType type, ProvideUnitPort* provider = NULL);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;

        inline BWAPI::UnitType getUnitType() const { return unitType; }

    protected:
        BWAPI::UnitType unitType;
};

class MoveToPositionPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        MoveToPositionPlanItem(ProvideUnitPort* provider, BWAPI::Position p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates(Time current) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        inline BWAPI::Position getPosition() const { return position; }

    protected:
        BWAPI::Position position;
};


class AttackUnitPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        RequireEnemyUnitPort enemyUnit;

        AttackUnitPlanItem(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates(Time current) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
};

class BuildPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort        requireResources;
        RequireSpacePort    requireSpace;
        SupplyPort          supply;

        BuildPlanItem(Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;

        inline BWAPI::UnitType getUnitType() const { return unitType; }

    protected:
        BWAPI::UnitType     unitType;
};
