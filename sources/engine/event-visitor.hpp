#pragma once

class AbstractEvent;
class ActionEvent;
class FrameEvent;
class BroodwarEvent;
class FieldSeenEvent;

class UnitUpdateEvent;
class SimpleUnitUpdateEvent;
class CompleteUnitUpdateEvent;
class ResourceUpdateEvent;

class AbstractActionEvent;
class ResourcesConsumedEvent;

class AbstractEventVisitor
{
    public:
        virtual void visitActionEvent(ActionEvent* event) = 0;
        virtual void visitFrameEvent(FrameEvent* event) = 0;
        virtual void visitBroodwarEvent(BroodwarEvent* event) = 0;
        virtual void visitFieldSeenEvent(FieldSeenEvent* event) = 0;

        virtual void visitUnitUpdateEvent(UnitUpdateEvent* event) = 0;
        virtual void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) = 0;
        virtual void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) = 0;
        virtual void visitResourceUpdateEvent(ResourceUpdateEvent* event) = 0;

        virtual void visitAbstractActionEvent(AbstractActionEvent* event) = 0;
        virtual void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) = 0;
};

class BasicEventVisitor : public AbstractEventVisitor
{
    public:
        void visitActionEvent(ActionEvent* event) override;
        void visitFrameEvent(FrameEvent* event) override;
        void visitBroodwarEvent(BroodwarEvent* event) override;
        void visitFieldSeenEvent(FieldSeenEvent* event) override;

        void visitUnitUpdateEvent(UnitUpdateEvent* event) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitResourceUpdateEvent(ResourceUpdateEvent* event) override;

        void visitAbstractActionEvent(AbstractActionEvent* event) override;
        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;
};
