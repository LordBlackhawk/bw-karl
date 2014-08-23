#include "blackboard-fixture.hpp"
#include "engine/broodwar-events.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include <BWAPI.h>

MockupAction::MockupAction()
    : AbstractAction(NULL)
{ }

MockupAction::Status MockupAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    return Running;
}

MockupPlanItem::MockupPlanItem(int time)
    : AbstractSimpleUnitPlanItem(BWAPI::UnitTypes::Zerg_Drone)
{
    estimatedStartTime = time;
    provideUnit.estimatedTime = time;
}

void MockupPlanItem::acceptVisitor(AbstractVisitor* /*visitor*/)
{ }

void MockupPlanItem::updateEstimates(Time /*current*/)
{ }

AbstractAction* MockupPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* action = new MockupAction();
    engine->addAction(action);
    return action;
}

void MockupExecutionEngine::terminateAction(AbstractAction* /*action*/, bool /*cleanup*/)
{
    BOOST_REQUIRE( false && "Method 'terminateAction' should not be called" );
}

void MockupExecutionEngine::generateEvent(AbstractEvent* /*event*/)
{
    BOOST_REQUIRE( false && "Method 'generateEvent' should not be called" );
}

void MockupExecutionEngine::tick()
{
    BOOST_REQUIRE( false && "Method 'tick' should not be called" );
}

bool MockupExecutionEngine::isActive(AbstractAction* /*action*/) const
{
    BOOST_REQUIRE( false && "Method 'isActive' should not be called" );
    return false;
}

void MockupExecutionEngine::addAction(AbstractAction* action)
{
    actions.push_back(action);
}

AbstractEvent* MockupExecutionEngine::popEvent()
{
    if (events.empty())
        return NULL;

    AbstractEvent* event = events.front();
    events.pop_front();
    return event;
}

BlackboardFixture::BlackboardFixture()
    : blackboard(new Blackboard(&engine)), unitCounter(0), numMockupActions(MockupAction::objectsAlive), numMockupPlanItems(MockupPlanItem::objectsAlive)
{
    BWAPI::BWAPI_init();
    blackboard->getInformations()->lastUpdateTime = 0;
}

BlackboardFixture::~BlackboardFixture()
{
    delete blackboard;
    BOOST_CHECK_EQUAL( MockupAction::objectsAlive, numMockupActions );
    BOOST_CHECK_EQUAL( MockupPlanItem::objectsAlive, numMockupPlanItems );
}

void BlackboardFixture::tick()
{
    blackboard->tick();
}

void BlackboardFixture::addEvent(AbstractEvent* event)
{
    engine.events.push_back(event);
}

void BlackboardFixture::addEvent(const BWAPI::Event& event)
{
    addEvent(new BroodwarEvent(event));
}

AbstractAction* BlackboardFixture::popAction()
{
    if (engine.actions.empty())
        return NULL;

    AbstractAction* action = engine.actions.front();
    engine.actions.pop_front();
    return action;
}

BWAPI::Unit* BlackboardFixture::createUniqueUnit()
{
    return (BWAPI::Unit*) unitCounter++;
}

AbstractBoundaryItem* BlackboardFixture::createBoundaryItem(BWAPI::UnitType ut, BWAPI::Player* player, BWAPI::Position pos, BWAPI::TilePosition tp)
{
    BWAPI::Unit* unit = createUniqueUnit();
    addEvent(new CompleteUnitUpdateEvent(unit, ut, tp, pos, player));
    tick();
    auto result = blackboard->lookupUnit(unit);
    BOOST_REQUIRE( result != NULL );
    return result;
}

AbstractBoundaryItem* BlackboardFixture::createBoundaryItem(BWAPI::UnitType ut, BWAPI::Position pos)
{
    return createBoundaryItem(ut, NULL, pos, BWAPI::TilePositions::Unknown);
}

AbstractBoundaryItem* BlackboardFixture::createBoundaryItem(BWAPI::UnitType ut)
{
    return createBoundaryItem(ut, BWAPI::Positions::Unknown);
}

OwnUnitBoundaryItem* BlackboardFixture::createOwnUnitBoundaryItem(BWAPI::UnitType ut, BWAPI::Position pos)
{
    auto result = dynamic_cast<OwnUnitBoundaryItem*>(createBoundaryItem(ut, NULL, pos, BWAPI::TilePositions::Unknown));
    BOOST_REQUIRE( result != NULL );
    return result;
}

ResourceBoundaryItem* BlackboardFixture::createResourceBoundaryItem(BWAPI::TilePosition tp)
{
    auto result = dynamic_cast<ResourceBoundaryItem*>(createBoundaryItem(BWAPI::UnitTypes::Resource_Mineral_Field, NULL, BWAPI::Positions::Unknown, tp));
    BOOST_REQUIRE( result != NULL );
    return result;
}

void BlackboardFixture::destroyBoundaryItem(AbstractBoundaryItem* item)
{
    BWAPI::Unit* unit = item->unit;
    addEvent(BWAPI::Event::UnitDestroy(unit));
    tick();
    BOOST_REQUIRE( blackboard->lookupUnit(unit) == NULL );
}

void BlackboardFixture::setupFields()
{
    if (blackboard->getInformations()->fields.getWidth() > 0)
        return;

    blackboard->getInformations()->fields.resize(64, 64);
}

void BlackboardFixture::buildBaseLocations()
{
    setupFields();
    auto informations = blackboard->getInformations();
    for (int l=0; l<4; ++l) {
        auto base = new BaseLocation;
        for (int k=0; k<10; ++k)
            base->minerals.insert(createResourceBoundaryItem(BWAPI::TilePosition(k, k)));
        informations->allBaseLocations.insert(base);
        if (l % 2 == 0)
            informations->ownBaseLocations.insert(base);
    }
}

namespace
{
    class ProvideUnitPlanItem : public AbstractPlanItem
    {
        public:
            ProvideUnitPort provideUnit;

            ProvideUnitPlanItem(BWAPI::UnitType ut, BWAPI::Position pos, int time)
                : provideUnit(this, NULL)
            {
                provideUnit.updateData(ut, pos);
                provideUnit.estimatedTime = time;
                estimatedStartTime = time;
            }

            void acceptVisitor(AbstractVisitor* visitor) override
            {
                visitor->visitAbstractPlanItem(this);
            }

            void updateEstimates(Time /*current*/) override
            {
                // Do not change estimatedStartTime on tick!!!
            }

            AbstractAction* prepareForExecution(AbstractExecutionEngine* /*engine*/) override
            {
                return NULL;
            }

            void removeFinished(AbstractAction* /*action*/)
            { }
    };
}

ProvideUnitPort* BlackboardFixture::createProvideUnitPort(BWAPI::UnitType ut, Time estimatedStartTime, BWAPI::Position pos)
{
    return &(new ProvideUnitPlanItem(ut, pos, estimatedStartTime))->provideUnit;
}

void BlackboardFixture::finishPlanItem(AbstractPlanItem* item)
{
    item->removeFinished(NULL);
    blackboard->removeItem(item);
    delete item;
}
