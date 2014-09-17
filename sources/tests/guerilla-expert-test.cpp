#include "blackboard-fixture.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "expert/guerilla-expert.hpp"
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

BOOST_AUTO_TEST_SUITE_END()
