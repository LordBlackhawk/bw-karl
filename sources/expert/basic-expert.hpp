#pragma once

#include "plan/plan-item.hpp"
#include "plan/abstract-visitor.hpp"

class BasicExpert : public AbstractExpert, public AbstractVisitor
{
    public:
        BasicExpert();

        void tick(Blackboard* blackboard);

        virtual void visitAbstractPort(AbstractPort* port);
        void visitProvideUnitPort(ProvideUnitPort* port);
        void visitRequireUnitPort(RequireUnitPort* port);

        virtual void visitAbstractPlanItem(AbstractPlanItem* item);
        void visitOwnUnitPlanItem(OwnUnitPlanItem* item);
        void visitGatherMineralPlanItem(GatherMineralsPlanItem* item);

        virtual void prepare();

    protected:
        Blackboard* currentBlackboard;
};

class BasicPortExpert : public BasicExpert
{
    public:
        void visitAbstractPlanItem(AbstractPlanItem* item);
};

