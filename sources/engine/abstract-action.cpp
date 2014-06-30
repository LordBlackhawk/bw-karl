#include "abstract-action.hpp"
#include "event-visitor.hpp"

AbstractAction::AbstractAction(AbstractAction* pre)
    : precondition(pre)
{ }

void AbstractAction::onBegin(AbstractExecutionEngine* /*engine*/)
{ }

void AbstractAction::onEnd(AbstractExecutionEngine* /*engine*/)
{ }

ActionEvent::ActionEvent(AbstractAction* s, Type t)
    : sender(s), type(t)
{ }

void ActionEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitActionEvent(this);
}

void AbstractExecutionEngine::generateActionEvent(AbstractAction* action, ActionEvent::Type type)
{
    generateEvent(new ActionEvent(action, type));
}
