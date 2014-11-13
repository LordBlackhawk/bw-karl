#include "blackboard-fixture.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "expert/guerilla-expert.hpp"
#include "expert/secure-expert.hpp"
#include "utils/bw-helper.hpp"

BOOST_FIXTURE_TEST_SUITE( guerilla_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( basic_obstacle_solver )
{
    ObstacleSolver solver;
    solver.add(BWAPI::Position(80, 0), BWAPI::UnitTypes::Zerg_Zergling);
    solver.add(BWAPI::Position(-80, 0), BWAPI::UnitTypes::Zerg_Zergling);
    solver.add(BWAPI::Position(0, -80), BWAPI::UnitTypes::Zerg_Zergling);
    solver.add(BWAPI::Position(80, -80), BWAPI::UnitTypes::Zerg_Zergling);
    solver.add(BWAPI::Position(-80, -80), BWAPI::UnitTypes::Zerg_Zergling);
    solver.add(BWAPI::Position(80, 80), BWAPI::UnitTypes::Zerg_Zergling);
    solver.add(BWAPI::Position(-80, 80), BWAPI::UnitTypes::Zerg_Zergling);
    BWAPI::Position solution = solver.solve();
    BOOST_CHECK_EQUAL( solution, BWAPI::Position(0, 96) );
}

BOOST_AUTO_TEST_CASE( unaccessable_fields )
{
    ObstacleSolver solver;
    solver.add(BWAPI::Position(80, 0));
    solver.add(BWAPI::Position(-80, 0));
    solver.add(BWAPI::Position(0, -80));
    BWAPI::Position solution = solver.solve();
    BOOST_CHECK_EQUAL( solution, BWAPI::Position(0, 96) );
}

BOOST_AUTO_TEST_CASE( complete_guerilla_test )
{
    setupFields();

    auto ownUnit = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Position(10, 10));
    createEnemyUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Position(15, 15));
    createEnemyUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Position(15, 05));

    auto expert = new GuerillaExpert();
    blackboard->addExpert(new SecureExpert("GuerillaExpert", expert));
    tick();

    BOOST_REQUIRE( ownUnit->provideUnit.isConnected() );
    auto planItem = dynamic_cast<MoveToPositionPlanItem*>(ownUnit->provideUnit.getConnectedPort()->getOwner());
    BOOST_REQUIRE( planItem != NULL );
    BOOST_CHECK_EQUAL( planItem->creator, expert );
}

BOOST_AUTO_TEST_CASE( high_templer )
{
    auto unitType = BWAPI::UnitTypes::Protoss_High_Templar;
    auto weapon = unitType.groundWeapon();
    BOOST_CHECK_EQUAL(weapon.damageAmount(), 0);
    BOOST_CHECK_EQUAL(weapon.damageCooldown(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
