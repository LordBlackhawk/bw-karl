#pragma once

#include <boost/test/unit_test.hpp>
#include "plan/plan-item.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/abstract-visitor.hpp"
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

class MockupExecutionEngine : public AbstractExecutionEngine
{
    public:
        std::deque<AbstractEvent*>      events;
        std::deque<AbstractAction*>     actions;

        void terminateAction(AbstractAction* action, bool cleanup) override;
        void generateEvent(AbstractEvent* event) override;
        void addAction(AbstractAction* action) override;
        AbstractEvent* popEvent() override;
        void tick() override;
        bool isActive(AbstractAction* action) const override;
};

class BlackboardFixture 
{
    public:
        MockupExecutionEngine           engine;
        Blackboard*                     blackboard;
        int                             unitCounter;

        int numMockupActions;
        int numMockupPlanItems;

        BlackboardFixture();
        ~BlackboardFixture();

        template <class T>
        T* addItem(T* item)
        { blackboard->addItem(item); return item; }

        AbstractAction* popAction();
        void addEvent(AbstractEvent* event);
        void addEvent(const BWAPI::Event& event);
        void tick();

        BWAPI::Unit* createUniqueUnit();
        AbstractBoundaryItem* createBoundaryItem(BWAPI::UnitType ut, BWAPI::Player* player, BWAPI::Position pos, BWAPI::TilePosition tp);
        AbstractBoundaryItem* createBoundaryItem(BWAPI::UnitType ut, BWAPI::Position pos);
        AbstractBoundaryItem* createBoundaryItem(BWAPI::UnitType ut);
        OwnUnitBoundaryItem* createOwnUnitBoundaryItem(BWAPI::UnitType ut, BWAPI::Position pos = BWAPI::Positions::Unknown);
        ResourceBoundaryItem* createResourceBoundaryItem(BWAPI::TilePosition tp);
        void destroyBoundaryItem(AbstractBoundaryItem* item);
        void setupFields();
        void buildBaseLocations();

        MorphUnitPlanItem* morphUnit(BWAPI::UnitType ut);
};
