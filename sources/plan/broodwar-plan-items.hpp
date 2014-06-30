#pragma once

#include "broodwar-ports.hpp"

class OwnUnitPlanItem : public AbstractBoundaryItem
{
    public:
        OwnUnitPlanItem(BWAPI::Unit* u);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitUnitUpdateEvent(UnitUpdateEvent* event) override; 

        inline BWAPI::UnitType getUnitType() const { return provideUnit.getUnitType(); }

    protected:
        ProvideUnitPort provideUnit;
};

class AbstractSimpleUnitPlanItem : public AbstractPlanItem
{
    public:
        RequireUnitPort     requireUnit;
        ProvideUnitPort     provideUnit;

        AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od = false);

        void updateEstimates() override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
        void removeFinished(AbstractAction* action) override;
};

class GatherMineralsPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        GatherMineralsPlanItem(BWAPI::Unit* m, ProvideUnitPort* provider = NULL);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates() override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::Unit*        mineral;
};

class BuildPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort        resources;

        BuildPlanItem(BWAPI::UnitType ut, BWAPI::TilePosition p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates() override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::UnitType     unitType;
        BWAPI::TilePosition pos;
};
