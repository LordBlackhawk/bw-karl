#include <boost/test/unit_test.hpp>

#include "engine/default-execution-engine.hpp"
#include "engine/basic-actions.hpp"

#define CHECK_NOEVENT()     do { BOOST_REQUIRE( engine.popEvent() == NULL ); } while (false)

#define CHECK_EVENT(_type, _sender)                                                 \
    do { ActionEvent* event = dynamic_cast<ActionEvent*>(engine.popEvent());        \
         BOOST_REQUIRE(event != NULL);                                              \
         BOOST_REQUIRE_EQUAL( event->type, _type );                                 \
         BOOST_REQUIRE_EQUAL( event->sender, _sender );                             \
         delete event;                                                              \
         } while (false)

namespace
{
    class MockupAction : public AbstractAction
    {
        public:
            enum TestStatus { testCreated, testRunning, testFailed, testFinished };
            TestStatus testStatus;
            int remainingTicks;

            MockupAction(AbstractAction* pre, int ticks)
                : AbstractAction(pre), testStatus(testCreated), remainingTicks(ticks)
            { }

            void onBegin(AbstractExecutionEngine* /*engine*/)
            {
                BOOST_CHECK_EQUAL( testStatus, testCreated );
                testStatus = testRunning;
            }

            Status onTick(AbstractExecutionEngine* /*engine*/)
            {
                BOOST_CHECK_EQUAL( testStatus, testRunning );
                if (remainingTicks > 0)
                    return Running;
                --remainingTicks;
                testStatus = testFailed;
                return Failed;
            }

            void onEnd(AbstractExecutionEngine* /*engine*/)
            {
                BOOST_CHECK( (testStatus == testRunning) || (testStatus == testFailed) );
                testStatus = testFinished;
            }
    };

    class DefaultExecutionEngineFixture
    {
        public:
            ~DefaultExecutionEngineFixture()
            {
                CHECK_NOEVENT();
                for (auto it : actions)
                    delete it;
                actions.clear();
            }

            AbstractAction* add(AbstractAction* a)
            {
                actions.push_back(a);
                engine.addAction(a);
                return a;
            }

            void tick()
            {
                engine.tick();
            }

            bool isActive(AbstractAction* a) const
            {
                return engine.isActive(a);
            }

            DefaultExecutionEngine engine;
            std::vector<AbstractAction*> actions;

            AbstractAction* addInfiniteAction(AbstractAction* pre = NULL)
            {
                return add(new MockupAction(pre, 10000));
            }

            AbstractAction* addFailAction(AbstractAction* pre = NULL)
            {
                return add(new MockupAction(pre, 0));
            }
    };
}

BOOST_FIXTURE_TEST_SUITE( default_execution_engine, DefaultExecutionEngineFixture )

BOOST_AUTO_TEST_CASE( basic )
{
    auto inf = addInfiniteAction();
    tick();
    CHECK_NOEVENT();

    auto terminate = add(new TerminateAction(inf, false));
    tick();
    CHECK_EVENT(ActionEvent::ActionTerminated, inf);
    CHECK_EVENT(ActionEvent::ActionFinished, terminate);
}

BOOST_AUTO_TEST_CASE( terminate_cleanup )
{
    auto action1 = addInfiniteAction();
    auto action2 = addInfiniteAction(action1);
    auto terminate = add(new TerminateAction(action1, true));

    tick();

    CHECK_EVENT(ActionEvent::ActionTerminated, action1);
    CHECK_EVENT(ActionEvent::ActionCleanedUp, action2);
    CHECK_EVENT(ActionEvent::ActionFinished, terminate);
}

BOOST_AUTO_TEST_CASE( terminate_do_not_cleanup )
{
    auto action1 = addInfiniteAction();
    auto action2 = addInfiniteAction(action1);
    auto terminate = add(new TerminateAction(action1, false));

    tick();

    CHECK_EVENT(ActionEvent::ActionTerminated, action1);
    CHECK_EVENT(ActionEvent::ActionFinished, terminate);
    BOOST_CHECK(isActive(action2));
    BOOST_CHECK_EQUAL( engine.numberOfActions(), 1 );
    BOOST_CHECK_EQUAL( engine.numberOfActiveActions(), 1 );
    BOOST_CHECK(!engine.containsAction(action1));
    BOOST_CHECK(!engine.containsAction(terminate));
}

BOOST_AUTO_TEST_CASE( fail_action_cleanup )
{
    auto action = addFailAction();
    auto follow = addInfiniteAction(action);

    tick();

    CHECK_EVENT(ActionEvent::ActionFailed, action);
    CHECK_EVENT(ActionEvent::ActionCleanedUp, follow);
}

BOOST_AUTO_TEST_CASE( terminate_middle_action )
{
    auto action1 = addInfiniteAction();
    auto action2 = addInfiniteAction(action1);
    auto action3 = addInfiniteAction(action2);
    auto terminate = add(new TerminateAction(action2, false));

    tick();

    CHECK_EVENT(ActionEvent::ActionTerminated, action2);
    CHECK_EVENT(ActionEvent::ActionFinished, terminate);
    BOOST_CHECK(isActive(action1));
    BOOST_CHECK(!isActive(action3));
    BOOST_CHECK(action3->precondition == action1);
}

BOOST_AUTO_TEST_SUITE_END()
