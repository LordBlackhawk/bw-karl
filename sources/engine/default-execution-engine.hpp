#pragma once

#include "abstract-action.hpp"

#include <deque>
#include <set>
#include <vector>

struct ActionInfo
{
    enum Info { Passiv, Active, Removed };
    Info info;
    AbstractAction* action;

    ActionInfo(Info i, AbstractAction* a)
        : info(i), action(a)
    { }
};

class DefaultExecutionEngine : public AbstractExecutionEngine
{
    public:
        DefaultExecutionEngine();
        void terminateAction(AbstractAction* action, bool cleanup);
        void generateEvent(AbstractEvent* event);
        void addAction(AbstractAction* action);
        AbstractEvent* popEvent();
        void tick();
        bool isActive(AbstractAction* action) const;
        bool containsAction(AbstractAction* action) const;
        bool havePendingEvents() override;

        int numberOfActions() const { return actions.size(); }
        int numberOfActiveActions() const;

    protected:
        std::vector<ActionInfo>     actions;
        std::deque<AbstractEvent*>  events;

        std::vector<ActionInfo>::iterator findAction(AbstractAction* action);
        std::vector<ActionInfo>::const_iterator findAction(AbstractAction* action) const;
        void markFollowUps(AbstractAction* action, ActionInfo::Info info);
};
