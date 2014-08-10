#include "blackboard-fixture.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "expert/supply-expert.hpp"

BOOST_FIXTURE_TEST_SUITE( supply_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( basic )
{
    auto o1 = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Overlord);
    for (int k=0; k<7; ++k)
        createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Drone);

    auto p1 = morphUnit(BWAPI::UnitTypes::Zerg_Drone);
    auto p2 = morphUnit(BWAPI::UnitTypes::Zerg_Drone);

    BOOST_CHECK( p2->supply.isRequirePort() );
    BOOST_CHECK_EQUAL( p2->supply.getRequiredAmount(), 2U );
    BOOST_CHECK( !o1->supply.isRequirePort() );
    BOOST_CHECK_EQUAL( o1->supply.getProvidedAmount(), 16U );

    SupplyExpert expert;
    expert.tick(blackboard);

    BOOST_CHECK( p1->supply.isConnected() );
    BOOST_CHECK_EQUAL( p1->supply.estimatedTime, ACTIVE_TIME );
    BOOST_CHECK( !p2->supply.isConnected() );

    auto& items = blackboard->getItems();
    auto it = std::find_if(items.begin(), items.end(), [&] (AbstractPlanItem* i) {
                auto m = dynamic_cast<MorphUnitPlanItem*>(i);
                return (m != NULL) && (m->getUnitType() == BWAPI::UnitTypes::Zerg_Overlord);
            });
    BOOST_REQUIRE( it != items.end() );

    auto o2 = static_cast<MorphUnitPlanItem*>(*it);
    o2->supply.estimatedTime = 1000;

    expert.tick(blackboard);
    BOOST_CHECK( p2->supply.isConnected() );
    BOOST_CHECK_EQUAL( p2->supply.estimatedTime, 1000 );
}

BOOST_AUTO_TEST_CASE( two_in_one_egg )
{
    auto h = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Hatchery);
    auto z = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Zergling);
    auto m = morphUnit(BWAPI::UnitTypes::Zerg_Zergling);

    BOOST_CHECK_EQUAL( h->supply.getProvidedAmount(), 2U );
    BOOST_CHECK_EQUAL( z->supply.getRequiredAmount(), 1U );
    BOOST_CHECK_EQUAL( m->supply.getRequiredAmount(), 2U );
}

BOOST_AUTO_TEST_SUITE_END()
