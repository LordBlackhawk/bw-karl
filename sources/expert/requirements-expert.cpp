#include "requirements-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-boundary-items.hpp"

REGISTER_EXPERT(RequirementsExpert)

void RequirementsExpert::addProvider(BWAPI::UnitType ut, AbstractItem* item)
{
    auto it = firstProvider.find(ut);
    if (it != firstProvider.end())
        return;
    firstProvider[ut] = item;
}

void RequirementsExpert::visitRequireUnitExistancePort(RequireUnitExistancePort* port)
{
    requirePorts.push_back(port);
}

void RequirementsExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    // Recognize units which are building at the moment.
    if (item->provideUnit.isConnected()) {
        auto planitem = item->provideUnit.getConnectedPort()->getOwner();
        auto morph = dynamic_cast<MorphUnitPlanItem*>(planitem);
        if ((morph != NULL) && (morph->getUnitType() == item->getUnitType()) && morph->isActive())
            return;
        auto build = dynamic_cast<BuildPlanItem*>(planitem);
        if ((build != NULL) && (build->getUnitType() == item->getUnitType()) && build->isActive())
            return;
    }
    addProvider(item->getUnitType(), item);
}

void RequirementsExpert::visitMorphUnitPlanItem(MorphUnitPlanItem* item)
{
    BasicPortExpert::visitMorphUnitPlanItem(item);
    addProvider(item->getUnitType(), item);
}

void RequirementsExpert::visitBuildPlanItem(BuildPlanItem* item)
{
    BasicPortExpert::visitBuildPlanItem(item);
    addProvider(item->getUnitType(), item);
}

void RequirementsExpert::endTraversal()
{
    for (auto it : requirePorts) {
        auto provider = firstProvider.find(it->getUnitType());
        if (provider != firstProvider.end()) {
            if (!it->isConnected())
                it->connectTo(provider->second);
        } else {
            AbstractPlanItem* newprovider = currentBlackboard->create(it->getUnitType());
            if (newprovider != NULL) {
                firstProvider[it->getUnitType()] = newprovider;
                newprovider->addPurpose(it->connectTo(newprovider), dynamic_cast<AbstractPlanItem*>(it->getOwner()));
            }
        }
    }
    requirePorts.clear();
    firstProvider.clear();
}
