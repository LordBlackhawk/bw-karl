#pragma once

#include "abstract-action.hpp"

#include <deque>
#include <set>
#include <vector>

class DefaultExecutionEngine : public AbstractExecutionEngine
{
    public:
        void terminateAction(AbstractAction* action, bool cleanup);
        void generateEvent(AbstractAction* action, Event::Type type, int data);
        void addAction(AbstractAction* action);
        bool isActive(AbstractAction* action) const;
        Event getEvent();

        void tick();

    protected:
        std::set<AbstractAction*>   allActions;
        std::set<AbstractAction*>   passiveActions;
        std::set<AbstractAction*>   activeActions;
        std::deque<Event>           events;

        std::vector<AbstractAction*> findFollowUps(AbstractAction* action);
        void activateFollowUps(AbstractAction* action);
        void terminateFollowUps(AbstractAction* action);
};
