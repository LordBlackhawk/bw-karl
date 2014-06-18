#pragma once

#include "broodwar-ports.hpp"

class OwnUnitPlanItem : public AbstractPlanItem
{
    public:
        OwnUnitPlanItem(BWAPI::Unit* u);
        void updateData(BWAPI::UnitType ut, BWAPI::Position p);

        void acceptVisitor(AbstractVisitor* visitor);
        bool prepareForExecution(AbstractExecutionEngine* engine);

    protected:
        ProvideUnitPort provideUnit;
};

class GatherMineralsPlanItem : public AbstractPlanItem
{
    public:
        GatherMineralsPlanItem(BWAPI::Unit* m, ProvideUnitPort* provider = NULL);

        void acceptVisitor(AbstractVisitor* visitor);
        void updateEstimates();
        bool prepareForExecution(AbstractExecutionEngine* engine);

    protected:
        RequireUnitPort     requireUnit;
        ProvideUnitPort     provideUnit;
        BWAPI::Unit*        mineral;
};
