#include "blackboard-fixture.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "expert/requirements-expert.hpp"

namespace
{
    template <class T>
    int countPortsOfType(AbstractItem* item)
    {
        int result = 0;
        for (auto it : item->ports)
            if (dynamic_cast<T*>(it) != NULL)
                ++result;
        return result;
    }

    template <class T>
    T* findPortOfType(AbstractItem* item)
    {
        for (auto it : item->ports) {
            T* result = dynamic_cast<T*>(it);
            if (result != NULL)
                return result;
        }
        return NULL;
    }
}

BOOST_FIXTURE_TEST_SUITE( requirements_test, BlackboardFixture )

BOOST_AUTO_TEST_CASE( basic )
{
    auto pool = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    auto ling = blackboard->morph(BWAPI::UnitTypes::Zerg_Zergling);

    RequirementsExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE_EQUAL( countPortsOfType<RequireUnitExistancePort>(ling), 1 );
    auto requirePort = findPortOfType<RequireUnitExistancePort>(ling);
    BOOST_CHECK_EQUAL( requirePort->getUnitType(), BWAPI::UnitTypes::Zerg_Spawning_Pool );
    BOOST_CHECK( requirePort->isConnected() );

    BOOST_REQUIRE_EQUAL( countPortsOfType<ProvideUnitExistancePort>(pool), 1 );
    auto providePort = findPortOfType<ProvideUnitExistancePort>(pool);
    BOOST_CHECK( providePort->isConnected() );
    BOOST_CHECK( !providePort->isImpossible() );
}

BOOST_AUTO_TEST_CASE( create_provider )
{
    auto ling = blackboard->morph(BWAPI::UnitTypes::Zerg_Zergling);

    RequirementsExpert expert;
    expert.tick(blackboard);

    BOOST_REQUIRE_EQUAL( countPortsOfType<RequireUnitExistancePort>(ling), 1 );
    auto requirePort = findPortOfType<RequireUnitExistancePort>(ling);
    BOOST_CHECK_EQUAL( requirePort->getUnitType(), BWAPI::UnitTypes::Zerg_Spawning_Pool );
    BOOST_REQUIRE( requirePort->isConnected() );

    auto provider = dynamic_cast<BuildPlanItem*>(requirePort->getConnectedPort()->getOwner());
    BOOST_REQUIRE( provider != NULL );
    BOOST_CHECK_EQUAL( provider->getUnitType(), BWAPI::UnitTypes::Zerg_Spawning_Pool );
}

BOOST_AUTO_TEST_CASE( plan_item_finished )
{
    // Create situation where a spawning pool is at the moment been created:
    auto pool = createOwnUnitBoundaryItem(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    auto build = blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    build->requireUnit.connectTo(&pool->provideUnit);
    build->estimatedStartTime = 0;
    build->setActive();
    build->setExecuting();

    auto ling = blackboard->morph(BWAPI::UnitTypes::Zerg_Zergling);

    RequirementsExpert expert;
    expert.tick(blackboard);
    tick();

    BOOST_CHECK_EQUAL( countPortsOfType<ProvideUnitExistancePort>(pool), 0 );
    BOOST_REQUIRE_EQUAL( countPortsOfType<ProvideUnitExistancePort>(build), 1 );
    auto providePort = findPortOfType<ProvideUnitExistancePort>(build);
    BOOST_CHECK( providePort->isConnected() );
    BOOST_CHECK( !providePort->isImpossible() );
    BOOST_CHECK_EQUAL( providePort->estimatedDuration, BWAPI::UnitTypes::Zerg_Spawning_Pool.buildTime() );
    BOOST_CHECK_EQUAL( providePort->estimatedTime, BWAPI::UnitTypes::Zerg_Spawning_Pool.buildTime() );

    finishPlanItem(build);
    tick();

    BOOST_REQUIRE_EQUAL( countPortsOfType<RequireUnitExistancePort>(ling), 1 );
    auto requirePort = findPortOfType<RequireUnitExistancePort>(ling);
    BOOST_CHECK_EQUAL( requirePort->getUnitType(), BWAPI::UnitTypes::Zerg_Spawning_Pool );
    BOOST_CHECK( requirePort->isConnected() );
    BOOST_CHECK( !requirePort->isImpossible() );

    BOOST_REQUIRE_EQUAL( countPortsOfType<ProvideUnitExistancePort>(pool), 1 );
    BOOST_CHECK( findPortOfType<ProvideUnitExistancePort>(pool)->isConnected() );
}

BOOST_AUTO_TEST_SUITE_END()
