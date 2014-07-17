#include "mutex-execution-engine.hpp"
#include "utils/assert-throw.hpp"

MutexExecutionEngine::MutexExecutionEngine(AbstractExecutionEngine* e)
    : engine(e)
{ }

MutexExecutionEngine::~MutexExecutionEngine()
{
    delete engine;
}

void MutexExecutionEngine::exchangeInformations()
{
    Lock lock(mutex);
    while (AbstractEvent* event = engine->popEvent())
        events.push_back(event);
    for (auto it : actions)
        engine->addAction(it);
    actions.clear();
}

void MutexExecutionEngine::terminateAction(AbstractAction* /*action*/, bool /*cleanup*/)
{
    assert(false && "MutexExecutionEngine::terminateAction should not be called!");
}

void MutexExecutionEngine::generateEvent(AbstractEvent* event)
{
    Lock lock(mutex);
    events.push_back(event);
}

void MutexExecutionEngine::addAction(AbstractAction* action)
{
    Lock lock(mutex);
    actions.push_back(action);
}

AbstractEvent* MutexExecutionEngine::popEvent()
{
    Lock lock(mutex);
    if (events.empty())
        return NULL;
    auto result = events.back();
    events.pop_back();
    return result;
}

void MutexExecutionEngine::tick()
{
    exchangeInformations();
    engine->tick();
}

bool MutexExecutionEngine::isActive(AbstractAction* /*action*/) const
{
    assert(false && "MutexExecutionEngine::isActive should not be called!");
    return false;
}
