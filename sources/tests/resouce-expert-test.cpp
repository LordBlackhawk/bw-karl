#include "blackboard-fixture.hpp"
#include "expert/resource-expert.hpp"

BOOST_FIXTURE_TEST_SUITE( resource_expert_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( no_worker_infinte_time )
{
    auto a = addItem(new BuildPlanItem(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::TilePositions::Unknown));

    ResourceExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->resources.estimatedTime, INFINITE_TIME );
}

BOOST_AUTO_TEST_CASE( simple_worker )
{
    auto a = addItem(new BuildPlanItem(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::TilePositions::Unknown));
    auto b = addItem(new GatherMineralsPlanItem(NULL));
    auto c = addItem(new GatherMineralsPlanItem(NULL));

    b->setActive();
    c->setActive();

    ResourceExpert expert;
    expert.tick(blackboard);

    // 200 / (2 * 0.045) = 2222,222...
    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->resources.estimatedTime, 2222 );
}

BOOST_AUTO_TEST_CASE( delayed_worker )
{
    auto a = addItem(new BuildPlanItem(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::TilePositions::Unknown));
    auto b = addItem(new GatherMineralsPlanItem(NULL));
    auto c = addItem(new GatherMineralsPlanItem(NULL));

    b->setActive();
    c->estimatedStartTime = 1000;

    ResourceExpert expert;
    expert.tick(blackboard);

    // 1000 + (200 - 1000 * 0.045) / (2 * 0.045) = 2722,2222
    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->resources.estimatedTime, 2722 );
}

BOOST_AUTO_TEST_CASE( delayed_worker_diff_numbers )
{
    auto a = addItem(new BuildPlanItem(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::TilePositions::Unknown));
    auto b = addItem(new GatherMineralsPlanItem(NULL));
    auto c = addItem(new GatherMineralsPlanItem(NULL));

    b->estimatedStartTime = 10000;
    c->estimatedStartTime = 11000;

    ResourceExpert expert;
    expert.tick(blackboard);

    // last test case + 10000
    BOOST_REQUIRE( blackboard->includeItem(a) );
    BOOST_CHECK_EQUAL( a->resources.estimatedTime, 12722 );
}

BOOST_AUTO_TEST_SUITE_END()