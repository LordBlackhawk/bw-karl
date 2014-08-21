#include "giveup-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"

REGISTER_EXPERT(GiveUpExpert)

GiveUpExpert::GiveUpExpert() : enemyNearby(false)
{ }

bool GiveUpExpert::tick(Blackboard* blackboard)
{
    BasicExpert::tick(blackboard);

    if(enemyNearby)
    {
        blackboard->giveUp();
        return false;
    }
    return true;
}

void GiveUpExpert::endTraversal()
{

}

void GiveUpExpert::visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item)
{
        //for now just give up if enemy is attacking us ... 
    if(item->getPosition().isValid()
        && item->getPosition().getDistance(BWTA::getStartLocation(BWAPI::Broodwar->self())->getPosition())<32*20
        && item->getUnit()!=NULL && item->getUnit()->isAttacking() && !item->getUnitType().isWorker() )
    {
        enemyNearby=true;
    }
}