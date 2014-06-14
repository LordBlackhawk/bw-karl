#include "abstract-action.hpp"

AbstractAction::AbstractAction(AbstractAction* pre)
    : precondition(pre)
{ }

AbstractAction::~AbstractAction()
{ }

void AbstractAction::onBegin(AbstractExecutionEngine* /*engine*/)
{ }

void AbstractAction::onEnd(AbstractExecutionEngine* /*engine*/)
{ }
