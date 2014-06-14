#pragma once

#include <stddef.h>

class AbstractExecutionEngine;

class AbstractAction
{
    public:
        enum Status { Running, Finished, Failed };
        AbstractAction* precondition;

        AbstractAction(AbstractAction* pre);
        virtual ~AbstractAction();
        virtual void onBegin(AbstractExecutionEngine* engine);
        virtual Status onTick(AbstractExecutionEngine* engine) = 0;
        virtual void onEnd(AbstractExecutionEngine* engine);
};

class Event
{
    public:
        enum Type { NoEvent, ActionFinished, ActionFailed, ActionTerminated, ActionCleanedUp };

        AbstractAction* sender;
        Type            type;
        int             data;

        inline Event(AbstractAction* s, Type t, int d)
            : sender(s), type(t), data(d)
        { }

        inline Event()
            : sender(NULL), type(NoEvent), data(0)
        { }
};

class AbstractExecutionEngine
{
    public:
        // Interface to actions:
        virtual void terminateAction(AbstractAction* action, bool cleanup) = 0;
        virtual void generateEvent(AbstractAction* action, Event::Type type, int data) = 0;

        // Interface to environment:
        virtual void addAction(AbstractAction* action) = 0;
        virtual Event getEvent() = 0;
};
