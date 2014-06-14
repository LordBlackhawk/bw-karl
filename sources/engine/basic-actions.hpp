#pragma once

#include "abstract-action.hpp"

class InfiniteAction : public AbstractAction
{
    public:
        InfiniteAction(AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine);
};

class FailAction : public AbstractAction
{
    public:
        FailAction(AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine);
};

class TerminateAction : public AbstractAction
{
    public:
        TerminateAction(AbstractAction* a, bool c);
        Status onTick(AbstractExecutionEngine* engine);

    protected:
        AbstractAction* action;
        bool            cleanup;
};
