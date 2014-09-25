#pragma once

#include "abstract-action.hpp"
#include "utils/thread.hpp"
#include <vector>

class MutexExecutionEngine : public AbstractExecutionEngine
{
    public:
        MutexExecutionEngine(AbstractExecutionEngine* e);
        ~MutexExecutionEngine();
        void exchangeInformations();

        // Interface to actions:
        void terminateAction(AbstractAction* action, bool cleanup) override;
        void generateEvent(AbstractEvent* event) override;

        // Interface to environment:
        void addAction(AbstractAction* action) override;
        AbstractEvent* popEvent() override;
        void tick() override;
        bool isActive(AbstractAction* action) const override;
        bool havePendingEvents() override;

    private:
        AbstractExecutionEngine*        engine;
        Mutex                           mutex;
        std::vector<AbstractAction*>    actions;
        std::vector<AbstractEvent*>     events;
};
