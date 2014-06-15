#include <boost/test/unit_test.hpp>

#include "engine/default-execution-engine.hpp"
#include "engine/basic-actions.hpp"

#define CHECK_NOEVENT()     do { BOOST_REQUIRE_EQUAL( engine.getEvent().type, Event::NoEvent ); } while (false)
#define CHECK_EVENT(_type, _sender)                     \
    do { Event event = engine.getEvent();               \
         BOOST_REQUIRE_EQUAL( event.type, _type );      \
         BOOST_REQUIRE_EQUAL( event.sender, _sender );  \
         } while (false)

namespace
{
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
    };
}

BOOST_FIXTURE_TEST_SUITE( default_execution_engine, DefaultExecutionEngineFixture )

BOOST_AUTO_TEST_CASE( basic )
{
    auto inf = add(new InfiniteAction());
    tick();
    CHECK_NOEVENT();

    auto terminate = add(new TerminateAction(inf, false));
    tick();
    CHECK_EVENT(Event::ActionTerminated, inf);
    CHECK_EVENT(Event::ActionFinished, terminate);
}

BOOST_AUTO_TEST_CASE( terminate_cleanup )
{
    auto action1 = add(new InfiniteAction());
    auto action2 = add(new InfiniteAction(action1));
    auto terminate = add(new TerminateAction(action1, true));

    tick();

    CHECK_EVENT(Event::ActionTerminated, action1);
    CHECK_EVENT(Event::ActionCleanedUp, action2);
    CHECK_EVENT(Event::ActionFinished, terminate);
}

BOOST_AUTO_TEST_CASE( terminate_do_not_cleanup )
{
    auto action1 = add(new InfiniteAction());
    auto action2 = add(new InfiniteAction(action1));
    auto terminate = add(new TerminateAction(action1, false));

    tick();

    CHECK_EVENT(Event::ActionTerminated, action1);
    CHECK_EVENT(Event::ActionFinished, terminate);
    BOOST_CHECK(isActive(action2));
}

BOOST_AUTO_TEST_CASE( fail_action_cleanup )
{
    auto action = add(new FailAction());
    auto follow = add(new InfiniteAction(action));

    tick();

    CHECK_EVENT(Event::ActionFailed, action);
    CHECK_EVENT(Event::ActionCleanedUp, follow);
}

BOOST_AUTO_TEST_SUITE_END()
