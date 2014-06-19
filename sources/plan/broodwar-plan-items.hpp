#pragma once

#include "broodwar-ports.hpp"

class OwnUnitPlanItem : public AbstractPlanItem
{
    public:
        OwnUnitPlanItem(BWAPI::Unit* u);
        void updateData(BWAPI::UnitType ut, BWAPI::Position p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        bool prepareForExecution(AbstractExecutionEngine* engine) override;

    protected:
        ProvideUnitPort provideUnit;
};

class GatherMineralsPlanItem : public AbstractPlanItem
{
    public:
        GatherMineralsPlanItem(BWAPI::Unit* m, ProvideUnitPort* provider = NULL);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates() override;
        bool prepareForExecution(AbstractExecutionEngine* engine) override;

    protected:
        RequireUnitPort     requireUnit;
        ProvideUnitPort     provideUnit;
        BWAPI::Unit*        mineral;
};

class BuildPlanItem : public AbstractPlanItem
{
    public:
        BuildPlanItem(BWAPI::UnitType ut, BWAPI::TilePosition p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates() override;
        bool prepareForExecution(AbstractExecutionEngine* engine) override;

    protected:
        RequireUnitPort     requireUnit;
        ProvideUnitPort     provideUnit;
        BWAPI::UnitType     unitType;
        BWAPI::TilePosition pos;
};
