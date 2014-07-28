#include "blackboard-fixture.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "expert/worker-expert.hpp"

BOOST_FIXTURE_TEST_SUITE( worker_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( basic )
{
    buildBaseLocations();

    auto worker = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Drone, BWAPI::Position(1, 1));

    WorkerExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE( popAction() == NULL );
    tick();
    BOOST_REQUIRE( popAction() != NULL );

    BOOST_CHECK( worker->isConnected() );
    BOOST_REQUIRE_EQUAL( blackboard->getItems().size(), 1U );

    destroyBoundaryItem(worker);

    BOOST_REQUIRE_EQUAL( blackboard->getItems().size(), 1U );

    expert.tick(blackboard);

    BOOST_CHECK_EQUAL( blackboard->getItems().size(), 0U );
}

BOOST_AUTO_TEST_CASE( minerals_destroyed )
{
    setupFields();
    auto informations = blackboard->getInformations();
    auto mineral = createResourceBoundaryItem(BWAPI::TilePosition(3, 4));

    auto blocker = informations->fields[3][4].blocker;
    BOOST_REQUIRE( blocker != NULL );
    BOOST_CHECK( blocker->getOwner() == mineral );
    blocker = informations->fields[4][4].blocker;
    BOOST_REQUIRE( blocker != NULL );
    BOOST_CHECK( blocker->getOwner() == mineral );

    auto base = new BaseLocation;
    base->minerals.insert(mineral);
    informations->allBaseLocations.insert(base);
    informations->ownBaseLocations.insert(base);

    auto worker = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Drone, BWAPI::Position(1, 1));

    BOOST_CHECK_EQUAL( mineral->numberOfWorkers(), 0 );

    WorkerExpert expert;
    expert.tick(blackboard);

    BOOST_CHECK_EQUAL( mineral->numberOfWorkers(), 1 );
    BOOST_CHECK( worker->isConnected() );
    BOOST_REQUIRE_EQUAL( blackboard->getItems().size(), 1U );

    destroyBoundaryItem(mineral);

    BOOST_REQUIRE_EQUAL( blackboard->getItems().size(), 1U );

    expert.tick(blackboard);

    BOOST_CHECK_EQUAL( blackboard->getItems().size(), 0U );
}

BOOST_AUTO_TEST_SUITE_END()
