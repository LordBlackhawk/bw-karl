#include "blackboard-fixture.hpp"
#include "engine/broodwar-events.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "utils/bw-helper.hpp"

BOOST_FIXTURE_TEST_SUITE( blackboard_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( remove_after_finished )
{
    auto boundary = createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone);
    MockupPlanItem* a = addItem(new MockupPlanItem(0));
    MockupPlanItem* b = addItem(new MockupPlanItem(100));

    a->requireUnit.connectTo(boundary);
    b->requireUnit.connectTo(&a->provideUnit);

    tick();

    AbstractAction* action = popAction();
    BOOST_REQUIRE( action != NULL );
    addEvent(new ActionEvent(action, ActionEvent::ActionFinished));
    tick();

    BOOST_CHECK( !blackboard->includesItem(a) );
}

BOOST_AUTO_TEST_CASE( continue_after_failed )
{
    auto boundary = createProvideUnitPort(BWAPI::UnitTypes::Zerg_Drone);
    MockupPlanItem* a = addItem(new MockupPlanItem(0));
    MockupPlanItem* b = addItem(new MockupPlanItem(100));

    a->requireUnit.connectTo(boundary);
    b->requireUnit.connectTo(&a->provideUnit);

    tick();
    BOOST_CHECK( a->isActive() );

    AbstractAction* action = popAction();
    BOOST_REQUIRE( action != NULL );
    addEvent(new ActionEvent(action, ActionEvent::ActionFailed));
    tick();

    BOOST_REQUIRE( blackboard->includesItem(a) );
    BOOST_CHECK_EQUAL( a->getStatus(), AbstractPlanItem::Failed );
}

BOOST_AUTO_TEST_CASE( add_remove_unit )
{
    BWAPI::Unit* unit = NULL; // make sure the pointer is only used for identification.
    BWAPI::Player* player = NULL; // make sure the pointer is only used for identification.
    const BWAPI::UnitType ut = BWAPI::UnitTypes::Zerg_Zergling;
    const BWAPI::TilePosition tp = BWAPI::TilePositions::Unknown;
    const BWAPI::Position pos = BWAPI::Positions::Unknown;
    const int health = 100;

    addEvent(new CompleteUnitUpdateEvent(unit, ut, health, BWAction::Unknown, tp, pos, player));
    tick();
    BOOST_REQUIRE_EQUAL( blackboard->getBoundaries().size(), 1U );

    addEvent(BWAPI::Event::UnitDestroy(unit));
    tick();
    BOOST_CHECK_EQUAL( blackboard->getBoundaries().size(), 0U );
}

BOOST_AUTO_TEST_CASE( own_unit_update )
{
    const BWAPI::UnitType ut1 = BWAPI::UnitTypes::Zerg_Drone;
    const BWAPI::UnitType ut3 = BWAPI::UnitTypes::Zerg_Spawning_Pool;
    const BWAPI::UnitType ut4 = ut1;
    const BWAPI::Position pos1(5, 5);
    const BWAPI::Position pos2(10, 10);
    const BWAPI::Position pos3(15, 15);
    const BWAPI::Position pos4(20, 20);
    const BWAPI::TilePosition tp3(1, 1);
    const BWAPI::TilePosition tp4(2, 2);
    const int health = 100;

    setupFields();

    auto item = createOwnUnitBoundaryItem(ut1, pos1);
    auto unit = item->getUnit();
    BOOST_CHECK( !item->requireSpace.isConnected() );
    BOOST_CHECK_EQUAL( item->getPosition(), pos1 );

    addEvent(new SimpleUnitUpdateEvent(unit, pos2, health, BWAction::Unknown));
    tick();
    BOOST_CHECK( !item->requireSpace.isConnected() );
    BOOST_CHECK_EQUAL( item->getPosition(), pos2 );

    addEvent(new CompleteUnitUpdateEvent(unit, ut3, health, BWAction::Unknown, tp3, pos3, NULL));
    tick();
    BOOST_CHECK( item->requireSpace.isConnected() );
    BOOST_CHECK_EQUAL( item->getPosition(), pos3 );
    BOOST_CHECK_EQUAL( item->getTilePosition(), tp3 );

    addEvent(new CompleteUnitUpdateEvent(unit, ut4, health, BWAction::Unknown, tp4, pos4, NULL));
    tick();
    BOOST_CHECK( !item->requireSpace.isConnected() );
    BOOST_CHECK_EQUAL( item->getPosition(), pos4 );
}

BOOST_AUTO_TEST_CASE( hatchery_boundary_item )
{
    const BWAPI::UnitType ut1 = BWAPI::UnitTypes::Zerg_Drone;
    const BWAPI::UnitType ut2 = BWAPI::UnitTypes::Zerg_Hatchery;
    const BWAPI::UnitType ut3 = ut1;
    const BWAPI::Position pos1(5, 5);
    const BWAPI::Position pos2(10, 10);
    const BWAPI::Position pos3(15, 15);
    const BWAPI::TilePosition tp2(1, 1);
    const BWAPI::TilePosition tp3(2, 2);
    const int health = 100;

    setupFields();

    auto item = createOwnUnitBoundaryItem(ut1, pos1);
    auto unit = item->getUnit();

    addEvent(new CompleteUnitUpdateEvent(unit, ut2, health, BWAction::Unknown, tp2, pos2, NULL));
    tick();
    BOOST_CHECK( dynamic_cast<OwnHatcheryBoundaryItem*>(blackboard->lookupUnit(unit)) != NULL );

    addEvent(new CompleteUnitUpdateEvent(unit, ut3, health, BWAction::Unknown, tp3, pos3, NULL));
    tick();
    BOOST_CHECK( dynamic_cast<OwnHatcheryBoundaryItem*>(blackboard->lookupUnit(unit)) == NULL );
}

BOOST_AUTO_TEST_SUITE_END()
