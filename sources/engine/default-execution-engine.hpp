#pragma once

#include "abstract-action.hpp"

#include <deque>
#include <set>
#include <vector>

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

        int numberOfActions() const { return allActions.size(); }
        int numberOfActiveActions() const { return activeActions.size(); }

    protected:
        std::set<AbstractAction*>   allActions;
        std::set<AbstractAction*>   passiveActions;
        std::set<AbstractAction*>   activeActions;
        std::deque<AbstractEvent*>  events;

        std::vector<AbstractAction*> findFollowUps(AbstractAction* action);
        void setPreconditionOfFollowUps(AbstractAction* action, AbstractAction* newprecondition);
        void activateFollowUps(AbstractAction* action);
        void terminateFollowUps(AbstractAction* action);
};
