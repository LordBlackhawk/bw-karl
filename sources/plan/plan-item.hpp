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
        Time estimatedTime;

        AbstractPort(AbstractItem* o);

        inline AbstractItem* getOwner() const { return owner; }
        inline bool isImpossible() const { return isImpossibleTime(estimatedTime); }
        inline bool operator < (const AbstractPort& rhs) const { return estimatedTime < rhs.estimatedTime; }

        bool isActive() const;

        virtual ~AbstractPort();
        virtual bool isActiveConnection() const = 0;
        virtual bool isRequirePort() const = 0;
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;
        virtual void disconnect() = 0;
        virtual void updateEstimates() = 0;

    protected:
        AbstractItem*   owner;
};

class AbstractItem : public BasicEventVisitor
{
    public:
        std::vector<AbstractPort*>  ports;

        virtual ~AbstractItem();
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;

        bool isPortRegistered(AbstractPort* port);
        void removePort(AbstractPort* port);
        void update(AbstractEvent* event);

        bool isBoundaryItem() const;
        bool isPlanItem() const;
};

class AbstractBoundaryItem : public AbstractItem
{
    public:
        BWAPI::Unit* unit;

        AbstractBoundaryItem(BWAPI::Unit* u);

        inline BWAPI::Unit* getUnit() const { return unit; }
};

class AbstractPlanItem : public AbstractItem
{
    public:
        enum Status { Planned, Active, Executing, Failed };

        Time estimatedStartTime;

        AbstractPlanItem();

        inline bool isPlanned() const { return (status == Planned); }
        inline bool isActive() const { return (status == Active) || (status == Executing); }
        inline bool isImpossible() const { return isImpossibleTime(estimatedStartTime); }
        inline bool operator < (const AbstractPlanItem& rhs) const { return estimatedStartTime < rhs.estimatedStartTime; }
        inline Status getStatus() const { return status; }

        void setActive();
        void setErrorState(AbstractAction* action);

        virtual void updateEstimates(Time current);
        virtual AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) = 0;
        virtual void removeFinished(AbstractAction* action) = 0;

    protected:
        Status status;
};

class AbstractExpert
{
    public:
        virtual ~AbstractExpert() = default;
        virtual void prepare() = 0;
        virtual bool tick(Blackboard* blackboard) = 0; // returns false if it should be removed.
};

class ProvideUnitPort;

class ResourceBoundaryItem;
class EnemyUnitBoundaryItem;

class BuildPlanItem;
class MorphUnitPlanItem;
class GatherMineralsPlanItem;
class MoveToPositionPlanItem;
class AttackUnitPlanItem;
class GiveUpPlanItem;

class Blackboard : public BasicEventVisitor
{
    public:
        Blackboard(AbstractExecutionEngine* e);
        virtual ~Blackboard();

        static void sendFrameEvent(AbstractExecutionEngine* engine);

        inline const std::vector<AbstractPlanItem*>& getItems() const { return items; }
        inline const std::map<BWAPI::Unit*, AbstractBoundaryItem*>& getBoundaries() const { return unitBoundaries; }
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

        void visitActionEvent(ActionEvent* event) override;
        void visitFrameEvent(FrameEvent* event) override;
        void visitBroodwarEvent(BroodwarEvent* event) override;
        void visitUnitUpdateEvent(UnitUpdateEvent* event) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitCreepChangedEvent(CreepChangedEvent* event) override;
        void visitAbstractActionEvent(AbstractActionEvent* event) override;

        // for creation of plan items:
        BuildPlanItem* build(BWAPI::UnitType ut);
        MorphUnitPlanItem* morph(BWAPI::UnitType ut);
        GatherMineralsPlanItem* gather(ProvideUnitPort* provider, ResourceBoundaryItem* m);
        MoveToPositionPlanItem* move(ProvideUnitPort* provider, BWAPI::Position p);
        MoveToPositionPlanItem* move(ProvideUnitPort* provider, BWAPI::TilePosition tp);
        AttackUnitPlanItem* attack(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy);
        GiveUpPlanItem* giveUp();

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
