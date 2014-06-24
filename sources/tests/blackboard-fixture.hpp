#pragma once

#include <boost/test/unit_test.hpp>
#include "plan/plan-item.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "engine/abstract-action.hpp"
#include "utils/object-counter.hpp"
#include <deque>

class MockupAction : public AbstractAction, public ObjectCounter<MockupAction>
{
    public:
        MockupAction();
        Status onTick(AbstractExecutionEngine* engine) override;
};

class MockupPlanItem : public AbstractSimpleUnitPlanItem, public ObjectCounter<MockupPlanItem>
{
    public:
        MockupPlanItem(int time);
        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates() override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
};

class BlackboardFixture : public AbstractExecutionEngine
{
    public:
        Blackboard*                     blackboard;
        std::deque<AbstractEvent*>      events;
        std::deque<AbstractAction*>     actions;

        int numMockupActions;
        int numMockupPlanItems;

        BlackboardFixture();
        ~BlackboardFixture();

        void terminateAction(AbstractAction* action, bool cleanup) override;
        void generateEvent(AbstractEvent* event) override;
        void addAction(AbstractAction* action) override;
        AbstractAction* popAction();
        AbstractEvent* popEvent() override;
        void tick() override;
        bool isActive(AbstractAction* action) const override;

        template <class T>
        T* addItem(T* item)
        { blackboard->addItem(item); return item; }
};
