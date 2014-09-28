#pragma once

#include "broodwar-ports.hpp"

class ResourceBoundaryItem;

class AbstractSimpleUnitPlanItem : public AbstractPlanItem
{
    public:
        RequireUnitPort     requireUnit;
        ProvideUnitPort     provideUnit;

        AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od = false);

        void removeFinished(AbstractAction* action) override;

    protected:
        void addRequirements(BWAPI::UnitType ut);
        void bridgeUnitExistancePortsTo(AbstractItem* item);
        void removeRequireExistancePorts();
};

class GatherResourcesPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        RequireResourcePort requireResource;

        GatherResourcesPlanItem(ResourceBoundaryItem* m, ProvideUnitPort* provider);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* buildAction() override;

        bool isGatherMinerals() const;
        inline bool isGatherGas() const { return !isGatherMinerals(); }
};

class MorphUnitPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort requireResources;
        SupplyPort supply;

        MorphUnitPlanItem(BWAPI::UnitType type, ProvideUnitPort* provider = NULL);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* buildAction() override;
        void removeFinished(AbstractAction* action) override;

        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;

        inline BWAPI::UnitType getUnitType() const { return unitType; }

    protected:
        BWAPI::UnitType unitType;
};

class MoveToPositionPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        bool useSmartTurnAround;

        MoveToPositionPlanItem(ProvideUnitPort* provider, BWAPI::Position p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates(Time current) override;
        AbstractAction* buildAction() override;

        inline BWAPI::Position getPosition() const { return position; }

    protected:
        BWAPI::Position position;
};

class AttackPositionPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        AttackPositionPlanItem(ProvideUnitPort* provider, BWAPI::Position p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates(Time current) override;
        AbstractAction* buildAction() override;

        inline BWAPI::Position getPosition() const { return position; }

    protected:
        BWAPI::Position position;
};

class AttackUnitPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        RequireEnemyUnitPort enemyUnit;

        AttackUnitPlanItem(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void updateEstimates(Time current) override;
        AbstractAction* buildAction() override;
};

class BuildPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort        requireResources;
        RequireSpacePort    requireSpace;
        SupplyPort          supply;

        BuildPlanItem(Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* buildAction() override;
        void removeFinished(AbstractAction* action) override;

        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;

        inline BWAPI::UnitType getUnitType() const { return unitType; }

    protected:
        BWAPI::UnitType     unitType;
};

class ResearchTechPlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort        requireResources;

        ResearchTechPlanItem(BWAPI::TechType t);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* buildAction() override;

        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;

        inline BWAPI::TechType getTechType() const { return tech; }

    protected:
        BWAPI::TechType     tech;
};

class UpgradePlanItem : public AbstractSimpleUnitPlanItem
{
    public:
        ResourcePort        requireResources;

        UpgradePlanItem(BWAPI::UpgradeType u, int l = 1);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* buildAction() override;

        void visitResourcesConsumedEvent(ResourcesConsumedEvent* event) override;

        inline BWAPI::UpgradeType getUpgradeType() const { return upgrade; }

    protected:
        BWAPI::UpgradeType  upgrade;
        int                 level;
};


class GiveUpPlanItem : public AbstractPlanItem
{
    public:
        GiveUpPlanItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* buildAction() override;
        void removeFinished(AbstractAction* /*action*/) override;

    protected:
};
