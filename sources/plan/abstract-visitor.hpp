#pragma once

class ProvideUnitPort;
class RequireUnitPort;

class OwnUnitPlanItem;
class GatherMineralsPlanItem;
class BuildPlanItem;

class AbstractVisitor
{
    public:
        virtual void visitProvideUnitPort(ProvideUnitPort* port) = 0;
        virtual void visitRequireUnitPort(RequireUnitPort* port) = 0;

        virtual void visitOwnUnitPlanItem(OwnUnitPlanItem* item) = 0;
        virtual void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) = 0;
        virtual void visitBuildPlanItem(BuildPlanItem* item) = 0;
};
