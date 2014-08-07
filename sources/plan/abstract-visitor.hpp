#pragma once

class AbstractPort;
class ProvideUnitPort;
class RequireUnitPort;
class ResourcePort;
class ProvideMineralFieldPort;
class RequireMineralFieldPort;
class RequireSpacePort;

class ProvideEnemyUnitPort;
class RequireEnemyUnitPort;

class AbstractBoundaryItem;
class OwnUnitBoundaryItem;
class ResourceBoundaryItem;
class EnemyUnitBoundaryItem;

class AbstractPlanItem;
class GatherMineralsPlanItem;
class MoveToPositionPlanItem;
class BuildPlanItem;

class AttackUnitPlanItem;

class AbstractVisitor
{
    public:
        virtual void visitProvideUnitPort(ProvideUnitPort* port) = 0;
        virtual void visitRequireUnitPort(RequireUnitPort* port) = 0;
        virtual void visitResourcePort(ResourcePort* port) = 0;
        virtual void visitProvideMineralFieldPort(ProvideMineralFieldPort* port) = 0;
        virtual void visitRequireMineralFieldPort(RequireMineralFieldPort* port) = 0;
        virtual void visitRequireSpacePort(RequireSpacePort* port) = 0;
		
		virtual void visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port) = 0;
		virtual void visitRequireEnemyUnitPort(RequireEnemyUnitPort* port) = 0;

        virtual void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) = 0;
        virtual void visitResourceBoundaryItem(ResourceBoundaryItem* item) = 0;
        virtual void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) = 0;

        virtual void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) = 0;
        virtual void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) = 0;
        virtual void visitBuildPlanItem(BuildPlanItem* item) = 0;
		
		virtual void visitAttackUnitPlanItem(AttackUnitPlanItem* item) = 0;

};

class BasicVisitor : public AbstractVisitor
{
    public:
        virtual void visitAbstractPort(AbstractPort* port);
        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;
        void visitResourcePort(ResourcePort* port) override;
        void visitProvideMineralFieldPort(ProvideMineralFieldPort* port) override;
        void visitRequireMineralFieldPort(RequireMineralFieldPort* port) override;
        void visitRequireSpacePort(RequireSpacePort* port) override;
		
		void visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port) override;
		void visitRequireEnemyUnitPort(RequireEnemyUnitPort* port) override;
        
        virtual void visitAbstractBoundaryItem(AbstractBoundaryItem* item);
        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitResourceBoundaryItem(ResourceBoundaryItem* item) override;
        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override;

        virtual void visitAbstractPlanItem(AbstractPlanItem* item);
        void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) override;
        void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) override;
        void visitBuildPlanItem(BuildPlanItem* item) override;
		
		void visitAttackUnitPlanItem(AttackUnitPlanItem* item) override;
};
