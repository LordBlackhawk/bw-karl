#include "blackboard-fixture.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "expert/require-unit-expert.hpp"

BOOST_FIXTURE_TEST_SUITE( larva_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( basic_larva )
{
    auto b1 = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Larva);
    createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Larva);
    createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Larva);

    auto p1 = blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
    auto p2 = blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);

    BOOST_CHECK_EQUAL( b1->provideUnit.getUnitType(), BWAPI::UnitTypes::Zerg_Larva );
    BOOST_CHECK_EQUAL( p1->requireUnit.getUnitType(), BWAPI::UnitTypes::Zerg_Larva );
    BOOST_CHECK( !p1->isActive() );

    RequireUnitExpert expert;
    expert.tick(blackboard);

    BOOST_CHECK( p1->requireUnit.isConnected() );
    BOOST_CHECK( p2->requireUnit.isConnected() );
}

BOOST_AUTO_TEST_CASE( active_plan_items )
{
    createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Larva);
    auto b1 = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Larva);
    createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Larva);

    auto p1 = blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
    auto p2 = blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);

    BOOST_CHECK( !b1->provideUnit.isActiveConnection() );   
    BOOST_CHECK( !p1->requireUnit.isActiveConnection() );

    p1->requireUnit.connectTo(&b1->provideUnit);
    p1->setActive();

    BOOST_CHECK( b1->provideUnit.isActiveConnection() );
    BOOST_CHECK( p1->requireUnit.isActiveConnection() );

    RequireUnitExpert expert;
    expert.tick(blackboard);

    BOOST_CHECK( b1->provideUnit.isConnected() );
    BOOST_CHECK( p1->requireUnit.isConnected() );
    BOOST_CHECK( b1->provideUnit.getConnectedPort() == &p1->requireUnit );

    BOOST_CHECK( p2->requireUnit.isConnected() );
}

BOOST_AUTO_TEST_CASE( two_provider )
{
    auto pool = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);

    auto upgrade = blackboard->upgrade(BWAPI::UpgradeTypes::Metabolic_Boost);

    RequireUnitExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE( upgrade->requireUnit.isConnected() );
    BOOST_CHECK( upgrade->requireUnit.getConnectedPort()->getOwner() == pool );
}

BOOST_AUTO_TEST_CASE( no_provider )
{
    auto upgrade = blackboard->upgrade(BWAPI::UpgradeTypes::Metabolic_Boost);

    RequireUnitExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE( upgrade->requireUnit.isConnected() );
    auto planItem = dynamic_cast<BuildPlanItem*>(upgrade->requireUnit.getConnectedPort()->getOwner());
    BOOST_REQUIRE( planItem != NULL );
    BOOST_CHECK_EQUAL( planItem->getUnitType(), BWAPI::UnitTypes::Zerg_Spawning_Pool );
}

BOOST_AUTO_TEST_SUITE_END()