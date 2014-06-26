#pragma once

class AbstractPort;
class ProvideUnitPort;
class RequireUnitPort;
class ResourcePort;

class AbstractPlanItem;
class OwnUnitPlanItem;
class GatherMineralsPlanItem;
class BuildPlanItem;

class AbstractVisitor
{
    public:
        virtual void visitProvideUnitPort(ProvideUnitPort* port) = 0;
        virtual void visitRequireUnitPort(RequireUnitPort* port) = 0;
        virtual void visitResourcePort(ResourcePort* port) = 0;

        virtual void visitOwnUnitPlanItem(OwnUnitPlanItem* item) = 0;
        virtual void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) = 0;
        virtual void visitBuildPlanItem(BuildPlanItem* item) = 0;
};

class BasicVisitor : public AbstractVisitor
{
    public:
        virtual void visitAbstractPort(AbstractPort* port);
        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;
        void visitResourcePort(ResourcePort* port) override;

        virtual void visitAbstractPlanItem(AbstractPlanItem* item);
        void visitOwnUnitPlanItem(OwnUnitPlanItem* item) override;
        void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) override;
        void visitBuildPlanItem(BuildPlanItem* item) override;
};
