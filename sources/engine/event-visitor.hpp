#pragma once

class AbstractEvent;
class ActionEvent;
class FrameEvent;
class BroodwarEvent;
class CreepChangedEvent;

class UnitUpdateEvent;
class SimpleUnitUpdateEvent;
class CompleteUnitUpdateEvent;
class MineralUpdateEvent;

class AbstractEventVisitor
{
    public:
        virtual void visitActionEvent(ActionEvent* event) = 0;
        virtual void visitFrameEvent(FrameEvent* event) = 0;
        virtual void visitBroodwarEvent(BroodwarEvent* event) = 0;
        virtual void visitCreepChangedEvent(CreepChangedEvent* event) = 0;

        virtual void visitUnitUpdateEvent(UnitUpdateEvent* event) = 0;
        virtual void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) = 0;
        virtual void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) = 0;
        virtual void visitMineralUpdateEvent(MineralUpdateEvent* event) = 0;
};

class BasicEventVisitor : public AbstractEventVisitor
{
    public:
        void visitActionEvent(ActionEvent* event) override;
        void visitFrameEvent(FrameEvent* event) override;
        void visitBroodwarEvent(BroodwarEvent* event) override;
        void visitCreepChangedEvent(CreepChangedEvent* event) override;

        void visitUnitUpdateEvent(UnitUpdateEvent* event) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitMineralUpdateEvent(MineralUpdateEvent* event) override;
};
