#pragma once

#include "broodwar-ports.hpp"

class OwnUnitPlanItem : public AbstractPlanItem
{
    public:
        OwnUnitPlanItem(BWAPI::Unit* u);
        void updateData(BWAPI::UnitType ut, BWAPI::Position p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
        void removeFinished(AbstractAction* action) override;

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
        BuildPlanItem(BWAPI::UnitType ut, BWAPI::TilePosition p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates() override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::UnitType     unitType;
        BWAPI::TilePosition pos;
};
