#include "basic-actions.hpp"

InfiniteAction::InfiniteAction(AbstractAction* pre)
    : AbstractAction(pre)
{ }

InfiniteAction::Status InfiniteAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    return Running;
}

FailAction::FailAction(AbstractAction* pre)
    : AbstractAction(pre)
{ }

FailAction::Status FailAction::onTick(AbstractExecutionEngine* /*engine*/)
{
    return Failed;
}

TerminateAction::TerminateAction(AbstractAction* a, bool c)
    : AbstractAction(NULL), action(a), cleanup(c)
{ }

TerminateAction::Status TerminateAction::onTick(AbstractExecutionEngine* engine)
{
    engine->terminateAction(action, cleanup);
    return Finished;
}
