
#include "broodwar-plan-items.hpp"

#pragma once

class AbstractPort;
class ProvideUnitPort;
class RequireUnitPort;
class ResourcePort;
class SupplyPort;
class ProvideResourcePort;
class RequireResourcePort;
class RequireSpacePort;
class ProvideEnemyUnitPort;
class RequireEnemyUnitPort;
class ProvideUnitExistancePort;
class RequireUnitExistancePort;
class ProvidePurposePort;
class RequirePurposePort;

class AbstractBoundaryItem;
class OwnUnitBoundaryItem;
class OwnHatcheryBoundaryItem;
class ResourceBoundaryItem;
class EnemyUnitBoundaryItem;

class AbstractPlanItem;
class GatherResourcesPlanItem;
class MorphUnitPlanItem;
class MoveToPositionPlanItem;
class BuildPlanItem;
class AttackUnitPlanItem;
class AttackPositionPlanItem;
class ResearchTechPlanItem;
class UpgradePlanItem;

class AbstractVisitor
{
    public:
        virtual void visitAbstractPort(AbstractPort* port) = 0;
        virtual void visitProvideUnitPort(ProvideUnitPort* port) = 0;
        virtual void visitRequireUnitPort(RequireUnitPort* port) = 0;
        virtual void visitResourcePort(ResourcePort* port) = 0;
        virtual void visitSupplyPort(SupplyPort* port) = 0;
        virtual void visitProvideResourcePort(ProvideResourcePort* port) = 0;
        virtual void visitRequireResourcePort(RequireResourcePort* port) = 0;
        virtual void visitRequireSpacePort(RequireSpacePort* port) = 0;
        virtual void visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port) = 0;
        virtual void visitRequireEnemyUnitPort(RequireEnemyUnitPort* port) = 0;
        virtual void visitProvideUnitExistancePort(ProvideUnitExistancePort* port) = 0;
        virtual void visitRequireUnitExistancePort(RequireUnitExistancePort* port) = 0;
        virtual void visitProvidePurposePort(ProvidePurposePort* port) = 0;
        virtual void visitRequirePurposePort(RequirePurposePort* port) = 0;

        virtual void visitAbstractBoundaryItem(AbstractBoundaryItem* item) = 0;
        virtual void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) = 0;
        virtual void visitOwnHatcheryBoundaryItem(OwnHatcheryBoundaryItem* item) = 0;
        virtual void visitResourceBoundaryItem(ResourceBoundaryItem* item) = 0;
        virtual void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) = 0;

        virtual void visitAbstractPlanItem(AbstractPlanItem* item) = 0;
        virtual void visitGatherResourcesPlanItem(GatherResourcesPlanItem* item) = 0;
        virtual void visitMorphUnitPlanItem(MorphUnitPlanItem* item) = 0;
        virtual void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) = 0;
        virtual void visitBuildPlanItem(BuildPlanItem* item) = 0;
        virtual void visitAttackUnitPlanItem(AttackUnitPlanItem* item) = 0;
        virtual void visitAttackPositionPlanItem(AttackPositionPlanItem* item) = 0;
        virtual void visitGiveUpPlanItem(GiveUpPlanItem* item) = 0;
        virtual void visitResearchTechPlanItem(ResearchTechPlanItem* item) = 0;
        virtual void visitUpgradePlanItem(UpgradePlanItem* item) = 0;
};

class BasicVisitor : public AbstractVisitor
{
    public:
        void visitAbstractPort(AbstractPort* port) override;
        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;
        void visitResourcePort(ResourcePort* port) override;
        void visitSupplyPort(SupplyPort* port) override;
        void visitProvideResourcePort(ProvideResourcePort* port) override;
        void visitRequireResourcePort(RequireResourcePort* port) override;
        void visitRequireSpacePort(RequireSpacePort* port) override;
        void visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port) override;
        void visitRequireEnemyUnitPort(RequireEnemyUnitPort* port) override;
        void visitProvideUnitExistancePort(ProvideUnitExistancePort* port) override;
        void visitRequireUnitExistancePort(RequireUnitExistancePort* port) override;
        void visitProvidePurposePort(ProvidePurposePort* port) override;
        void visitRequirePurposePort(RequirePurposePort* port) override;
        
        void visitAbstractBoundaryItem(AbstractBoundaryItem* item) override;
        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitOwnHatcheryBoundaryItem(OwnHatcheryBoundaryItem* item) override;
        void visitResourceBoundaryItem(ResourceBoundaryItem* item) override;
        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override;

        void visitAbstractPlanItem(AbstractPlanItem* item) override;
        void visitGatherResourcesPlanItem(GatherResourcesPlanItem* item) override;
        void visitMorphUnitPlanItem(MorphUnitPlanItem* item) override;
        void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) override;
        void visitBuildPlanItem(BuildPlanItem* item) override;
        void visitAttackUnitPlanItem(AttackUnitPlanItem* item) override;
        void visitAttackPositionPlanItem(AttackPositionPlanItem* item) override;
        void visitGiveUpPlanItem(GiveUpPlanItem* item) override;
        void visitResearchTechPlanItem(ResearchTechPlanItem* item) override;
        void visitUpgradePlanItem(UpgradePlanItem* item) override;
};
