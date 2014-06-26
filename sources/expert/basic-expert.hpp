#pragma once

#include "plan/plan-item.hpp"
#include "plan/abstract-visitor.hpp"

class BasicExpert : public AbstractExpert, public BasicVisitor
{
    public:
        BasicExpert();

        void prepare() override;
        bool tick(Blackboard* blackboard) override;

        virtual void beginTraversal();
        virtual void endTraversal();

        static bool isApplicable(Blackboard* blackboard);

    protected:
        Blackboard* currentBlackboard;
};

class BasicPortExpert : public BasicExpert
{
    public:
        BasicPortExpert();

        void visitAbstractPlanItem(AbstractPlanItem* item) override;

    protected:
        AbstractPlanItem* currentPlanItem;
};
