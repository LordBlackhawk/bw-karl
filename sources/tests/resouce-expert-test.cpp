#include "blackboard-fixture.hpp"
#include "expert/resource-expert.hpp"
#include "plan/broodwar-boundary-items.hpp"

BOOST_FIXTURE_TEST_SUITE( resource_expert_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( no_worker_infinte_time )
{
    auto a = blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);

    ResourceExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->requireResources.estimatedTime, INFINITE_TIME );
}

BOOST_AUTO_TEST_CASE( simple_worker )
{
    setupFields();

    auto a = blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    auto m = createResourceBoundaryItem(BWAPI::TilePosition(5, 5));
    auto b = blackboard->gather(createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone), m);
    auto c = blackboard->gather(createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone), m);

    // execute tick to be sure that plan items are active and estimatedStartTime is correct:
    tick();
    BOOST_CHECK(b->isActive());
    BOOST_CHECK(c->isActive());

    ResourceExpert expert;
    expert.tick(blackboard);

    // 200 / (2 * 0.045) = 2222,222...
    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->requireResources.estimatedTime, 2223 );
}

BOOST_AUTO_TEST_CASE( delayed_worker )
{
    setupFields();

    auto a = blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    auto m = createResourceBoundaryItem(BWAPI::TilePosition(5, 5));
    auto b = blackboard->gather(createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone), m);
    auto c = blackboard->gather(createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone, 1000), m);

    // execute tick to be sure that plan items are active and estimatedStartTime is correct:
    tick();
    BOOST_CHECK(b->isActive());
    BOOST_CHECK_EQUAL(b->estimatedStartTime, 1);
    BOOST_CHECK(!c->isActive());
    BOOST_CHECK_EQUAL(c->estimatedStartTime, 1000);

    ResourceExpert expert;
    expert.tick(blackboard);

    // 1 + 1000 + (200 - 1000 * 0.045) / (2 * 0.045) = 2723,2222
    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->requireResources.estimatedTime, 2722 );
}

BOOST_AUTO_TEST_CASE( delayed_worker_diff_numbers )
{
    setupFields();

    auto a = blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    auto m = createResourceBoundaryItem(BWAPI::TilePosition(5, 5));
    blackboard->gather(createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone, 10000), m);
    blackboard->gather(createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone, 11000), m);

    // execute tick to be sure that plan items are active and estimatedStartTime is correct:
    tick();

    ResourceExpert expert;
    expert.tick(blackboard);

    // last test case + 10000
    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->requireResources.estimatedTime, 12722 );
}

BOOST_AUTO_TEST_SUITE_END()
