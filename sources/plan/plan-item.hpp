#pragma once

#include "blackboard-informations.hpp"
#include "engine/event-visitor.hpp"

#include <vector>
#include <set>

class AbstractAction;
class AbstractExecutionEngine;
class AbstractVisitor;
class Blackboard;

class AbstractPort
{
    public:
        Time estimatedTime;

        AbstractPort();

        inline bool isActive() const { return (estimatedTime == ACTIVE_TIME); }
        inline bool isImpossible() const { return isImpossibleTime(estimatedTime); }
        inline bool operator < (const AbstractPort& rhs) const { return estimatedTime < rhs.estimatedTime; }

        virtual ~AbstractPort();
        virtual bool isActiveConnection() const = 0;
        virtual bool isRequirePort() const = 0;
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;
};

class AbstractPlanItem
{
    public:
        Time                        estimatedStartTime;
        std::vector<AbstractPort*>  ports;

        AbstractPlanItem();

        inline bool isActive() const { return (estimatedStartTime == ACTIVE_TIME); }
        inline bool isImpossible() const { return isImpossibleTime(estimatedStartTime); }
        inline bool operator < (const AbstractPlanItem& rhs) const { return estimatedStartTime < rhs.estimatedStartTime; }

        void setActive();

        virtual ~AbstractPlanItem();
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;
        virtual void updateEstimates();
        virtual AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) = 0;
        virtual void removeFinished(AbstractAction* action) = 0;
};

class AbstractExpert
{
    public:
        virtual ~AbstractExpert();
        virtual void prepare() = 0;
        virtual void tick(Blackboard* blackboard) = 0;
};

class Blackboard : public AbstractEventVisitor
{
    public:
        Blackboard(AbstractExecutionEngine* e);
        virtual ~Blackboard();
    
        inline const std::vector<AbstractPlanItem*>& getItems() const { return items; }
        inline BlackboardInformations* getInformations() { return &informations; }
        inline Time getLastUpdateTime() const { return informations.lastUpdateTime; }
        inline BWAPI::Player* self() const { return informations.self; }

        void addItem(AbstractPlanItem* item);
        void removeItem(AbstractPlanItem* item);
        bool includeItem(AbstractPlanItem* item); // for test propose only

        void addExpert(AbstractExpert* expert);
        void removeExpert(AbstractExpert* expert);

        void prepare();
        void recalculateEstimatedTimes();
        void tick();

        void visitActionEvent(ActionEvent* event);
        void visitFrameEvent(FrameEvent* event);
        void visitBroodwarEvent(BroodwarEvent* event);
        void visitUnitEvent(UnitEvent* event);

    protected:
        AbstractExecutionEngine*                        engine;
        std::vector<AbstractPlanItem*>                  items;
        std::vector<AbstractExpert*>                    experts;
        BlackboardInformations                          informations;
        std::map<BWAPI::Unit*, AbstractPlanItem*>       unitBoundaries;
        std::map<AbstractAction*, AbstractPlanItem*>    actionMap;
};
