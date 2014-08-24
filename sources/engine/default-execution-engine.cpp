#include "default-execution-engine.hpp"
#include <algorithm>

DefaultExecutionEngine::DefaultExecutionEngine()
{ }

std::vector<ActionInfo>::iterator DefaultExecutionEngine::findAction(AbstractAction* action)
{
    return std::find_if(actions.begin(), actions.end(), [&] (const ActionInfo& i) { return i.action == action; });
}

std::vector<ActionInfo>::const_iterator DefaultExecutionEngine::findAction(AbstractAction* action) const
{
    return std::find_if(actions.begin(), actions.end(), [&] (const ActionInfo& i) { return i.action == action; });
}

void DefaultExecutionEngine::terminateAction(AbstractAction* action, bool cleanup)
{
    auto it = findAction(action);
    if (it == actions.end())
        return;

    if (it->info == ActionInfo::Active)
        it->action->onEnd(this);
    it->info = ActionInfo::Removed;
    generateActionEvent(action, ActionEvent::ActionTerminated);

    ActionInfo::Info newinfo = ActionInfo::Removed;
    if (!cleanup)
        newinfo = (it->action->precondition == NULL) ? ActionInfo::Active : ActionInfo::Passiv;
    markFollowUps(it->action, newinfo);
}

void DefaultExecutionEngine::generateEvent(AbstractEvent* event)
{
    events.push_back(event);
}

void DefaultExecutionEngine::addAction(AbstractAction* action)
{
    ActionInfo::Info info = ActionInfo::Passiv;
    if ((action->precondition == NULL) || !containsAction(action->precondition)) {
        info = ActionInfo::Active;
        action->precondition = NULL;
        action->onBegin(this);
    }
    actions.push_back(ActionInfo(info, action));
}

bool DefaultExecutionEngine::isActive(AbstractAction* action) const
{
    auto it = findAction(action);
    return (it != actions.end()) && (it->info == ActionInfo::Active);
}

bool DefaultExecutionEngine::containsAction(AbstractAction* action) const
{
    return (findAction(action) != actions.end());
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
    for (auto& it : actions)
        if (it.info == ActionInfo::Active)
    {
        AbstractAction::Status status = it.action->onTick(this);
        if (status != AbstractAction::Status::Running) {
            it.action->onEnd(this);
            it.info = ActionInfo::Removed;
            if (status == AbstractAction::Status::Finished) {
                generateActionEvent(it.action, ActionEvent::ActionFinished);
                markFollowUps(it.action, ActionInfo::Active);
            } else if (status == AbstractAction::Status::Failed) {
                generateActionEvent(it.action, ActionEvent::ActionFailed);
                markFollowUps(it.action, ActionInfo::Removed);
            }
        }
    }

    actions.erase(std::remove_if(actions.begin(), actions.end(), [] (const ActionInfo& i) { return i.info == ActionInfo::Removed; }), actions.end());
}

void DefaultExecutionEngine::markFollowUps(AbstractAction* action, ActionInfo::Info info)
{
    for (auto& it : actions)
        if (it.action->precondition == action)
    {
        it.info = info;
        switch (info) {
            case ActionInfo::Active:
                it.action->precondition = NULL;
                it.action->onBegin(this);
                break;
            case ActionInfo::Passiv:
                it.action->precondition = action->precondition;
                break;
            case ActionInfo::Removed:
                generateActionEvent(it.action, ActionEvent::ActionCleanedUp);
                markFollowUps(it.action, info);
                break;
        }
    }
}

int DefaultExecutionEngine::numberOfActiveActions() const
{
    return std::count_if(actions.begin(), actions.end(), [] (const ActionInfo& i) { return i.info == ActionInfo::Active; });
}
