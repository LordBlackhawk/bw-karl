#include "default-execution-engine.hpp"

DefaultExecutionEngine::DefaultExecutionEngine()
{ }

void DefaultExecutionEngine::terminateAction(AbstractAction* action, bool cleanup)
{
    if (allActions.find(action) == allActions.end())
        return;

    allActions.erase(action);
    if (action->precondition != NULL) {
        passiveActions.erase(action);
    } else {
        action->onEnd(this);
        activeActions.erase(action);
    }
    generateEvent(action, Event::ActionTerminated, 0);

    if (cleanup) {
        terminateFollowUps(action);
    } else {
        activateFollowUps(action);
    }
}

void DefaultExecutionEngine::generateEvent(AbstractAction* action, Event::Type type, int data)
{
    events.push_back(Event(action, type, data));
}

void DefaultExecutionEngine::addAction(AbstractAction* action)
{
    allActions.insert(action);
    if ((action->precondition != NULL) && (allActions.find(action->precondition) != allActions.end())) {
        passiveActions.insert(action);
    } else {
        action->precondition = NULL;
        activeActions.insert(action);
    }
}

bool DefaultExecutionEngine::isActive(AbstractAction* action) const
{
    return activeActions.find(action) != activeActions.end();
}

Event DefaultExecutionEngine::getEvent()
{
    if (events.empty())
        return Event();

    Event event = events.front();
    events.pop_front();
    return event;
}

void DefaultExecutionEngine::tick()
{
    std::set<AbstractAction*> copy = activeActions;
    for (auto it : copy) {
        AbstractAction::Status status = it->onTick(this);
        if (status == AbstractAction::Status::Finished) {
            allActions.erase(it);
            activeActions.erase(it);
            generateEvent(it, Event::ActionFinished, 0);
            activateFollowUps(it);
        } else if (status == AbstractAction::Status::Failed) {
            allActions.erase(it);
            activeActions.erase(it);
            generateEvent(it, Event::ActionFailed, 0);
            terminateFollowUps(it);
        }
    }
}

std::vector<AbstractAction*> DefaultExecutionEngine::findFollowUps(AbstractAction* action)
{
    std::vector<AbstractAction*> result;
    for (auto it : passiveActions)
        if (it->precondition == action)
            result.push_back(it);
    return result;
}

void DefaultExecutionEngine::activateFollowUps(AbstractAction* action)
{
    std::vector<AbstractAction*> result = findFollowUps(action);
    for (auto it : result) {
        passiveActions.erase(it);
        it->onBegin(this);
        activeActions.insert(it);
    }
}

void DefaultExecutionEngine::terminateFollowUps(AbstractAction* action)
{
    std::vector<AbstractAction*> all = findFollowUps(action);
    while (!all.empty()) {
        AbstractAction* current = all.back();
        allActions.erase(current);
        passiveActions.erase(current);
        generateEvent(current, Event::ActionCleanedUp, 0);

        all.pop_back();
        std::vector<AbstractAction*> additional = findFollowUps(current);
        all.insert(all.end(), additional.begin(), additional.end());
    }
}
