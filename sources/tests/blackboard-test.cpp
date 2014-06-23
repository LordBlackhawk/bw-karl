#include <boost/test/unit_test.hpp>

#include "plan/plan-item.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "engine/abstract-action.hpp"
#include "utils/object-counter.hpp"

#include <deque>

namespace
{
    class MockupAction : public AbstractAction, public ObjectCounter<MockupAction>
    {
        public:
            MockupAction()
                : AbstractAction(NULL)
            { }

            Status onTick(AbstractExecutionEngine* /*engine*/)
            {
                return Running;
            }
    };

    class MockupPlanItem : public AbstractSimpleUnitPlanItem, public ObjectCounter<MockupPlanItem>
    {
        public:
            MockupPlanItem(int time)
                : AbstractSimpleUnitPlanItem(BWAPI::UnitTypes::Zerg_Drone)
            {
                estimatedStartTime = time;
            }

            void acceptVisitor(AbstractVisitor* /*visitor*/)
            { }

            void updateEstimates()
            { }

            AbstractAction* prepareForExecution(AbstractExecutionEngine* engine)
            {
                AbstractAction* action = new MockupAction();
                engine->addAction(action);
                return action;
            }
    };

    class BlackboardFixture : public AbstractExecutionEngine
    {
        public:
            Blackboard*                     blackboard;
            std::deque<AbstractEvent*>      events;
            std::deque<AbstractAction*>     actions;

            int numMockupActions;
            int numMockupPlanItems;

            BlackboardFixture()
                : blackboard(new Blackboard(this)), numMockupActions(MockupAction::objectsAlive), numMockupPlanItems(MockupPlanItem::objectsAlive)
            { }

            ~BlackboardFixture()
            {
                delete blackboard;
                BOOST_CHECK_EQUAL( MockupAction::objectsAlive, numMockupActions );
                BOOST_CHECK_EQUAL( MockupPlanItem::objectsAlive, numMockupPlanItems );
            }

            void terminateAction(AbstractAction* /*action*/, bool /*cleanup*/) override
            {
                BOOST_REQUIRE( false && "Method 'terminateAction' should not be called" );
            }

            void generateEvent(AbstractEvent* /*event*/) override
            {
                BOOST_REQUIRE( false && "Method 'generateEvent' should not be called" );
            }

            void addAction(AbstractAction* action) override
            {
                actions.push_back(action);
            }
            
            AbstractAction* popAction()
            {
                if (actions.empty())
                    return NULL;

                AbstractAction* action = actions.front();
                actions.pop_front();
                return action;
            }

            AbstractEvent* popEvent() override
            {
                if (events.empty())
                    return NULL;

                AbstractEvent* event = events.front();
                events.pop_front();
                return event;
            }

            void tick() override
            {
                blackboard->tick();
            }

            bool isActive(AbstractAction* /*action*/) const override
            {
                BOOST_REQUIRE( false && "Method 'isActive' should not be called" );
                return false;
            }
    };
}

BOOST_FIXTURE_TEST_SUITE( blackboard_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( remove_after_finished )
{
    MockupPlanItem* a = new MockupPlanItem(-1);
    MockupPlanItem* b = new MockupPlanItem(5);
    MockupPlanItem* c = new MockupPlanItem(1000);

    b->requireUnit.connectTo(&a->provideUnit);
    c->requireUnit.connectTo(&b->provideUnit);

    blackboard->addItem(a);
    blackboard->addItem(b);
    blackboard->addItem(c);
    tick();

    BOOST_REQUIRE_EQUAL(actions.size(), 1U);
    AbstractAction* action = popAction();
    events.push_back(new ActionEvent(action, ActionEvent::ActionFinished));
    tick();

    BOOST_CHECK( !blackboard->includeItem(b) );
}

BOOST_AUTO_TEST_CASE( continue_after_failed )
{
    MockupPlanItem* a = new MockupPlanItem(-1);
    MockupPlanItem* b = new MockupPlanItem(5);
    MockupPlanItem* c = new MockupPlanItem(1000);

    b->requireUnit.connectTo(&a->provideUnit);
    c->requireUnit.connectTo(&b->provideUnit);

    blackboard->addItem(a);
    blackboard->addItem(b);
    blackboard->addItem(c);
    tick();

    BOOST_REQUIRE_EQUAL(actions.size(), 1U);
    AbstractAction* action = popAction();
    events.push_back(new ActionEvent(action, ActionEvent::ActionFailed));
    tick();

    BOOST_REQUIRE( blackboard->includeItem(b) );
    BOOST_CHECK_EQUAL( b->estimatedStartTime, INFINITE_TIME );
}

BOOST_AUTO_TEST_SUITE_END()
