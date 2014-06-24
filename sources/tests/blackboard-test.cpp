#include "blackboard-fixture.hpp"

BOOST_FIXTURE_TEST_SUITE( blackboard_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( remove_after_finished )
{
    MockupPlanItem* a = addItem(new MockupPlanItem(-1));
    MockupPlanItem* b = addItem(new MockupPlanItem(5));
    MockupPlanItem* c = addItem(new MockupPlanItem(1000));

    b->requireUnit.connectTo(&a->provideUnit);
    c->requireUnit.connectTo(&b->provideUnit);

    tick();

    BOOST_REQUIRE_EQUAL(actions.size(), 1U);
    AbstractAction* action = popAction();
    events.push_back(new ActionEvent(action, ActionEvent::ActionFinished));
    tick();

    BOOST_CHECK( !blackboard->includeItem(b) );
}

BOOST_AUTO_TEST_CASE( continue_after_failed )
{
    MockupPlanItem* a = addItem(new MockupPlanItem(-1));
    MockupPlanItem* b = addItem(new MockupPlanItem(5));
    MockupPlanItem* c = addItem(new MockupPlanItem(1000));

    b->requireUnit.connectTo(&a->provideUnit);
    c->requireUnit.connectTo(&b->provideUnit);

    tick();

    BOOST_REQUIRE_EQUAL(actions.size(), 1U);
    AbstractAction* action = popAction();
    events.push_back(new ActionEvent(action, ActionEvent::ActionFailed));
    tick();

    BOOST_REQUIRE( blackboard->includeItem(b) );
    BOOST_CHECK_EQUAL( b->estimatedStartTime, INFINITE_TIME );
}

BOOST_AUTO_TEST_SUITE_END()
