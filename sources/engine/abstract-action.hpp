#pragma once

#include <stddef.h>

class AbstractExecutionEngine;
class AbstractEventVisitor;

class AbstractAction
{
    public:
        enum Status { Running, Finished, Failed };
        AbstractAction* precondition;

        AbstractAction(AbstractAction* pre);
        virtual ~AbstractAction() = default;
        virtual void onBegin(AbstractExecutionEngine* engine);
        virtual Status onTick(AbstractExecutionEngine* engine) = 0;
        virtual void onEnd(AbstractExecutionEngine* engine);
};

class AbstractEvent
{
    public:
        virtual ~AbstractEvent() = default;

        virtual void acceptVisitor(AbstractEventVisitor* visitor) = 0;
};

class ActionEvent : public AbstractEvent
{
    public:
        enum Type { ActionFinished, ActionFailed, ActionTerminated, ActionCleanedUp };
        AbstractAction* sender;
        Type            type;
        ActionEvent(AbstractAction* s, Type t);
        void acceptVisitor(AbstractEventVisitor* visitor);
};

class AbstractExecutionEngine
{
    public:
        virtual ~AbstractExecutionEngine() = default;

        // Interface to actions:
        virtual void terminateAction(AbstractAction* action, bool cleanup) = 0;
        virtual void generateEvent(AbstractEvent* event) = 0;

        void generateActionEvent(AbstractAction* action, ActionEvent::Type type);

        // Interface to environment:
        virtual void addAction(AbstractAction* action) = 0;
        virtual AbstractEvent* popEvent() = 0;
        virtual void tick() = 0;
        virtual bool isActive(AbstractAction* action) const = 0;
};
