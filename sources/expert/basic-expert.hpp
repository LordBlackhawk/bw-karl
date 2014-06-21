#pragma once

#include "plan/plan-item.hpp"
#include "plan/abstract-visitor.hpp"

class BasicExpert : public AbstractExpert, public AbstractVisitor
{
    public:
        BasicExpert();

        void prepare() override;
        void tick(Blackboard* blackboard) override;

        virtual void visitAbstractPort(AbstractPort* port);
        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;

        virtual void visitAbstractPlanItem(AbstractPlanItem* item);
        void visitOwnUnitPlanItem(OwnUnitPlanItem* item) override;
        void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) override;
        void visitBuildPlanItem(BuildPlanItem* item) override;

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
