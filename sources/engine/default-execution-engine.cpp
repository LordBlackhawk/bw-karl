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
    generateActionEvent(action, ActionEvent::ActionTerminated);

    if (cleanup) {
        terminateFollowUps(action);
    } else {
        activateFollowUps(action);
    }
}

void DefaultExecutionEngine::generateEvent(AbstractEvent* event)
{
    events.push_back(event);
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

AbstractEvent* DefaultExecutionEngine::popEvent()
{
    if (events.empty())
        return NULL;

    AbstractEvent* event = events.front();
    events.pop_front();
    return event;
}

void DefaultExecutionEngine::tick()
{
    std::set<AbstractAction*> copy = activeActions;
    for (auto it : copy) {
        AbstractAction::Status status = it->onTick(this);
        if (status == AbstractAction::Status::Finished) {
            it->onEnd(this);
            allActions.erase(it);
            activeActions.erase(it);
            generateActionEvent(it, ActionEvent::ActionFinished);
            activateFollowUps(it);
        } else if (status == AbstractAction::Status::Failed) {
            it->onEnd(this);
            allActions.erase(it);
            activeActions.erase(it);
            generateActionEvent(it, ActionEvent::ActionFailed);
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
        generateActionEvent(current, ActionEvent::ActionCleanedUp);

        all.pop_back();
        std::vector<AbstractAction*> additional = findFollowUps(current);
        all.insert(all.end(), additional.begin(), additional.end());
    }
}
