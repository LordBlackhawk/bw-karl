#pragma once

#include "blackboard-informations.hpp"
#include "engine/event-visitor.hpp"

#include <vector>
#include <set>

class AbstractItem;
class AbstractAction;
class AbstractExecutionEngine;
class AbstractVisitor;
class Blackboard;

class AbstractPort
{
    public:
        Time            estimatedTime;

        AbstractPort(AbstractItem* o);

        inline AbstractItem* getOwner() const { return owner; }
        inline bool isActive() const { return (estimatedTime == ACTIVE_TIME); }
        inline bool isImpossible() const { return isImpossibleTime(estimatedTime); }
        inline bool operator < (const AbstractPort& rhs) const { return estimatedTime < rhs.estimatedTime; }

        virtual ~AbstractPort() = default;
        virtual bool isActiveConnection() const = 0;
        virtual bool isRequirePort() const = 0;
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;

    protected:
        AbstractItem*   owner;
};

class AbstractItem
{
    public:
        std::vector<AbstractPort*>  ports;

        virtual ~AbstractItem() = default;
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;

        void removePort(AbstractPort* port);

        bool isBoundaryItem() const;
        bool isPlanItem() const;
};

class AbstractBoundaryItem : public AbstractItem, public BasicEventVisitor
{
    public:
        BWAPI::Unit* unit;

        AbstractBoundaryItem(BWAPI::Unit* u);

        void update(AbstractEvent* event);

        inline BWAPI::Unit* getUnit() const { return unit; }
};

class AbstractPlanItem : public AbstractItem
{
    public:
        Time estimatedStartTime;

        AbstractPlanItem();

        inline bool isActive() const { return (estimatedStartTime == ACTIVE_TIME); }
        inline bool isImpossible() const { return isImpossibleTime(estimatedStartTime); }
        inline bool operator < (const AbstractPlanItem& rhs) const { return estimatedStartTime < rhs.estimatedStartTime; }

        void setActive();
        void setErrorState(AbstractAction* action);

        virtual void updateEstimates();
        virtual AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) = 0;
        virtual void removeFinished(AbstractAction* action) = 0;
};

class AbstractExpert
{
    public:
        virtual ~AbstractExpert() = default;
        virtual void prepare() = 0;
        virtual bool tick(Blackboard* blackboard) = 0; // returns false if it should be removed.
};

class BuildPlanItem;

class Blackboard : public BasicEventVisitor
{
    public:
        Blackboard(AbstractExecutionEngine* e);
        virtual ~Blackboard();

        static void sendFrameEvent(AbstractExecutionEngine* engine);
    
        inline const std::vector<AbstractPlanItem*>& getItems() const { return items; }
        inline const std::map<BWAPI::Unit*, AbstractBoundaryItem*> getBoundaries() const { return unitBoundaries; }
        inline BlackboardInformations* getInformations() { return &informations; }
        inline Time getLastUpdateTime() const { return informations.lastUpdateTime; }
        inline BWAPI::Player* self() const { return informations.self; }
        inline BWAPI::Player* neutral() const { return informations.neutral; }

        void addItem(AbstractPlanItem* item);
        void removeItem(AbstractPlanItem* item);

        void addExpert(AbstractExpert* expert);
        void removeExpert(AbstractExpert* expert);

        void prepare();
        void recalculateEstimatedTimes();
        void tick();

        void visitActionEvent(ActionEvent* event);
        void visitFrameEvent(FrameEvent* event);
        void visitBroodwarEvent(BroodwarEvent* event);
        void visitUnitUpdateEvent(UnitUpdateEvent* event);
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event);
        void visitCreepChangedEvent(CreepChangedEvent* event);

        // for creation of plan items:
        BuildPlanItem* createBuildPlanItem(BWAPI::UnitType ut); 

        // for test propose only:
        bool includeItem(AbstractPlanItem* item) const;
        AbstractBoundaryItem* lookupUnit(BWAPI::Unit* unit) const;

    protected:
        AbstractExecutionEngine*                        engine;
        std::vector<AbstractPlanItem*>                  items;
        std::vector<AbstractExpert*>                    experts;
        BlackboardInformations                          informations;
        std::map<BWAPI::Unit*, AbstractBoundaryItem*>   unitBoundaries;
        std::map<AbstractAction*, AbstractPlanItem*>    actionMap;
};
