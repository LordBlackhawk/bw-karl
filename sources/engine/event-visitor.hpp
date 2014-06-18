#pragma once

class ActionEvent;
class FrameEvent;
class BroodwarEvent;
class UnitEvent;

class AbstractEventVisitor
{
    public:
        virtual void visitActionEvent(ActionEvent* event) = 0;
        virtual void visitFrameEvent(FrameEvent* event) = 0;
        virtual void visitBroodwarEvent(BroodwarEvent* event) = 0;
        virtual void visitUnitEvent(UnitEvent* event) = 0;
};
