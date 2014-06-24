#include "blackboard-fixture.hpp"
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
}

void MockupPlanItem::acceptVisitor(AbstractVisitor* /*visitor*/)
{ }

void MockupPlanItem::updateEstimates()
{ }

AbstractAction* MockupPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    AbstractAction* action = new MockupAction();
    engine->addAction(action);
    return action;
}

BlackboardFixture::BlackboardFixture()
    : blackboard(new Blackboard(this)), numMockupActions(MockupAction::objectsAlive), numMockupPlanItems(MockupPlanItem::objectsAlive)
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

void BlackboardFixture::terminateAction(AbstractAction* /*action*/, bool /*cleanup*/)
{
    BOOST_REQUIRE( false && "Method 'terminateAction' should not be called" );
}

void BlackboardFixture::generateEvent(AbstractEvent* /*event*/)
{
    BOOST_REQUIRE( false && "Method 'generateEvent' should not be called" );
}

void BlackboardFixture::addAction(AbstractAction* action)
{
    actions.push_back(action);
}

AbstractAction* BlackboardFixture::popAction()
{
    if (actions.empty())
        return NULL;

    AbstractAction* action = actions.front();
    actions.pop_front();
    return action;
}

AbstractEvent* BlackboardFixture::popEvent()
{
    if (events.empty())
        return NULL;

    AbstractEvent* event = events.front();
    events.pop_front();
    return event;
}

void BlackboardFixture::tick()
{
    blackboard->tick();
}

bool BlackboardFixture::isActive(AbstractAction* /*action*/) const
{
    BOOST_REQUIRE( false && "Method 'isActive' should not be called" );
    return false;
}
