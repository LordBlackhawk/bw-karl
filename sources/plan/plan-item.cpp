#include "plan-item.hpp"
#include "broodwar-boundary-items.hpp"
#include "broodwar-plan-items.hpp"
#include "engine/abstract-action.hpp"
#include "engine/broodwar-events.hpp"
#include "engine/broodwar-scanners.hpp"
#include "engine/basic-actions.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"
#include "utils/bw-helper.hpp"
#include <algorithm>
#include <typeinfo>

AbstractPort::AbstractPort(AbstractItem* o)
    : estimatedTime(INFINITE_TIME), owner(o)
{
    owner->ports.push_back(this);
}

AbstractPort::~AbstractPort()
{
    owner->removePort(this);
}

bool AbstractPort::isActive() const
{
    return owner->isBoundaryItem() || dynamic_cast<AbstractPlanItem*>(owner)->isActive();
}

AbstractAction* AbstractPort::prepareForExecution(AbstractExecutionEngine* /*engine*/)
{
    return NULL;
}

AbstractItem::AbstractItem(BWAPI::Unit* u)
    : unit(u)
{ }

AbstractItem::~AbstractItem()
{
    while (!ports.empty()) {
        AbstractPort* port = ports.front();
        port->disconnect();
        if (!ports.empty() && (port == ports.front()))
            delete port;
    }
}

bool AbstractItem::isPortRegistered(AbstractPort* port)
{
    return std::find(ports.begin(), ports.end(), port) != ports.end();
}

void AbstractItem::removePort(AbstractPort* port)
{
    ports.erase(std::remove(ports.begin(), ports.end(), port), ports.end());
}

bool AbstractItem::isBoundaryItem() const
{
    return (dynamic_cast<const AbstractBoundaryItem*>(this) != NULL);
}

bool AbstractItem::isPlanItem() const
{
    return (dynamic_cast<const AbstractPlanItem*>(this) != NULL);
}

void AbstractItem::update(AbstractEvent* event)
{
    event->acceptVisitor(this);
}

void AbstractItem::setUnit(BWAPI::Unit* u)
{
    assert(unit == NULL);
    unit = u;
}

ResourceCategorySet AbstractItem::getCategory() const
{
    for (auto it : ports) {
        ResourcePort* port = dynamic_cast<ResourcePort*>(it);
        if (port != NULL)
            return port->getCategory();
    }
    return ResourceCategorySet();
}

AbstractBoundaryItem::AbstractBoundaryItem(BWAPI::Unit* u)
    : AbstractItem(u)
{ }

void AbstractBoundaryItem::takeConnectionsFrom(AbstractBoundaryItem* /*other*/)
{ }

AbstractPlanItem::AbstractPlanItem()
    : estimatedStartTime(INFINITE_TIME), creator(NULL), status(Planned), action(NULL)
{ }

void AbstractPlanItem::updateEstimates(Time current)
{
    if (status == Planned || status == Active) {
        estimatedStartTime = current + 1;
        for (auto it : ports)
            if (it->isRequirePort())
        {
            it->updateEstimates();
            if ((status != Active) || !it->isImpossible())
                estimatedStartTime = std::max(estimatedStartTime, it->estimatedTime);
        }
    }
    for (auto it : ports)
        if (!it->isRequirePort())
    {
        it->updateEstimates();
    }
}

namespace
{
    AbstractAction* andAction(AbstractExecutionEngine* engine, AbstractAction* first, AbstractAction* second)
    {
        if (first == NULL)
            return second;
        if (second == NULL)
            return first;
        auto waitAction = new InfiniteAction(first);
        engine->addAction(waitAction);
        engine->addAction(new TerminateAction(waitAction, false, second));
        return waitAction;
    }
}

AbstractAction* AbstractPlanItem::prepareForExecution(AbstractExecutionEngine* engine)
{
    assert(action == NULL && unit == NULL);
    action = buildAction();
    if (action != NULL) {
        AbstractAction* pre = NULL;
        for (auto it : ports)
            if (it->isRequirePort())
                pre = andAction(engine, pre, it->prepareForExecution(engine));
        action->precondition = pre;
        engine->addAction(action);
    }
    return action;
}

void AbstractPlanItem::setActive()
{
    assert(status == Planned);
    status = Active;
}

void AbstractPlanItem::setExecuting()
{
    assert(status == Active);
    status = Executing;
}

void AbstractPlanItem::setTerminated(AbstractExecutionEngine* engine)
{
    if (status == Terminated)
        return;
    assert(isActive());
    status = Terminated;
    engine->addAction(new TerminateAction(action, false));
}

void AbstractPlanItem::setFinished()
{
    assert(isActive());
    status = Finished;
}

void AbstractPlanItem::setErrorState(AbstractAction* /*action*/)
{
    assert(isActive());
    status = Failed;
}

void AbstractPlanItem::setPlanned()
{
    assert(isFailed());
    status = Planned;
    action = NULL;
    unit = NULL;
}

void AbstractPlanItem::addPurpose(AbstractPort* port, AbstractPlanItem* item)
{
    auto newport = new RequirePurposePort(this, port);
    newport->connectTo(item);
}

Blackboard::Blackboard(AbstractExecutionEngine* e)
    : engine(e), activeExpert(NULL)
{ }

Blackboard::~Blackboard()
{
    for (auto it : items)
        delete it;
    for (auto it : experts)
        delete it;
}

void Blackboard::addItem(AbstractPlanItem* item)
{
    if (item->creator == NULL)
        item->creator = activeExpert;
    items.push_back(item);
}

void Blackboard::removeItem(AbstractPlanItem* item)
{
    assert(!item->isActive());
    items.erase(std::remove(items.begin(), items.end(), item), items.end());
    delete item;
}

void Blackboard::terminate(AbstractPlanItem* item)
{
    item->setTerminated(engine);
}

bool Blackboard::includesItem(AbstractPlanItem* item) const
{
    return (std::find(items.begin(), items.end(), item) != items.end());
}

void Blackboard::addExpert(AbstractExpert* expert)
{
    experts.push_back(expert);
}

void Blackboard::removeExpert(AbstractExpert* expert)
{
    experts.erase(std::remove(experts.begin(), experts.end(), expert), experts.end());
}

void Blackboard::setActiveExpert(AbstractExpert* expert)
{
    activeExpert = expert;
}

AbstractBoundaryItem* Blackboard::lookupUnit(BWAPI::Unit* unit) const
{
    auto it = unitBoundaries.find(unit);
    return (it != unitBoundaries.end()) ? it->second : NULL;
}

AbstractPlanItem* Blackboard::create(BWAPI::UnitType ut, ResourceCategorySet c)
{
    BWAPI::UnitType builderType = ut.whatBuilds().first;
    if (builderType.isWorker())
        return build(ut, c);
    if ((builderType == BWAPI::UnitTypes::Zerg_Larva) || (builderType.isBuilding() && (builderType.getRace() == BWAPI::Races::Zerg)))
        return morph(ut, c);
    assert(false && "Blackboard does not know how to create unit of this type.");
    return NULL;
}

BuildPlanItem* Blackboard::build(BWAPI::UnitType ut, ResourceCategorySet c)
{
    auto result = new BuildPlanItem(&informations, ut, c, BWAPI::TilePositions::Unknown);
    addItem(result);
    return result;
}

MorphUnitPlanItem* Blackboard::morph(BWAPI::UnitType ut, ResourceCategorySet c)
{
    auto result = new MorphUnitPlanItem(&informations, ut, c);
    addItem(result);
    return result;
}

GatherResourcesPlanItem* Blackboard::gather(ProvideUnitPort* provider, ResourceBoundaryItem* m)
{
    auto result = new GatherResourcesPlanItem(m, provider);
    addItem(result);
    return result;
}

MoveToPositionPlanItem* Blackboard::move(ProvideUnitPort* provider, BWAPI::Position p)
{
    auto result = new MoveToPositionPlanItem(provider, p);
    addItem(result);
    return result;
}

MoveToPositionPlanItem* Blackboard::move(ProvideUnitPort* provider, BWAPI::TilePosition tp)
{
    return move(provider, BWAPI::Position(tp));
}

AttackUnitPlanItem* Blackboard::attack(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy)
{
    auto result = new AttackUnitPlanItem(provider, enemy);
    addItem(result);
    return result;
}

AttackPositionPlanItem* Blackboard::attack(ProvideUnitPort* provider, BWAPI::Position p)
{
    auto result = new AttackPositionPlanItem(provider, p);
    addItem(result);
    return result;
}

ResearchTechPlanItem* Blackboard::research(BWAPI::TechType tech, ResourceCategorySet c)
{
    auto result = new ResearchTechPlanItem(&informations, tech, c);
    addItem(result);
    return result;
}

UpgradePlanItem* Blackboard::upgrade(BWAPI::UpgradeType upgrade, ResourceCategorySet c)
{
    auto result = new UpgradePlanItem(&informations, upgrade, c);
    addItem(result);
    return result;
}

GiveUpPlanItem* Blackboard::giveUp()
{
    auto result = new GiveUpPlanItem();
    addItem(result);
    return result;
}

namespace
{
    class PlanItemCompare
    {
        public:
            bool operator () (AbstractPlanItem* lhs, AbstractPlanItem* rhs) const
            {
                return *lhs < *rhs;
            }
    };
}

void Blackboard::recalculateEstimatedTimes()
{
    for (auto it : items)
        it->updateEstimates(informations.lastUpdateTime);
    std::sort(items.begin(), items.end(), PlanItemCompare());
}

void Blackboard::tick()
{
    // 1. Receive events
    AbstractEvent* event = engine->popEvent();
    if (event == NULL)
        return;
    while (event != NULL) {
        event->acceptVisitor(this);
        delete event;
        event = engine->popEvent();
    }

    // 2. Experts may change the plan
    experts.erase(std::remove_if(experts.begin(), experts.end(), [&] (AbstractExpert* expert)
            {
                recalculateEstimatedTimes();
                activeExpert = expert;
                bool remove = !expert->tick(this);
                activeExpert = NULL;
                if (remove) delete expert;
                return remove;
            }), experts.end());

    // 3. Recalculate estimatedTimes
    recalculateEstimatedTimes();

    // 4. Execute actions which are planned soon
    const Time timeHorizon = getActionHorizon();
    for (auto it : items)
        if (it->isPlanned() && (it->estimatedStartTime < timeHorizon))
    {
        AbstractAction* action = it->prepareForExecution(engine);
        if (action != NULL) {
            actionMap[action] = it;
            it->setActive();
        }
    }
}

void Blackboard::matchEnd()
{
    for(auto expert : experts)
        expert->matchEnd(this);
}

void Blackboard::prepare()
{
    informations.prepare();
    for (auto base : informations.allBaseLocations)
        for (auto mineral : base->minerals)
            unitBoundaries[mineral->getUnit()] = mineral;
    for (auto it : experts)
        it->prepare();
    engine->addAction(new FieldScannerAction());
}

namespace
{
    int getHealth(BWAPI::Unit* unit, BWAPI::UnitType unitType)
    {
        int result = unit->getHitPoints();
        if (unitType.getRace() == BWAPI::Races::Protoss)
            result += unit->getShields();
        return result;
    }
}

bool Blackboard::sendFrameEvent(AbstractExecutionEngine* engine)
{
    BWAPI::Player* self = BWAPI::Broodwar->self();
    engine->generateEvent(new FrameEvent(BWAPI::Broodwar->getFrameCount(), self->minerals(), self->gas(), self->gatheredMinerals(), self->gatheredGas()));

    for (auto event : BWAPI::Broodwar->getEvents()) {
        if ( (event.getType() == BWAPI::EventType::UnitCreate)
          || (event.getType() == BWAPI::EventType::UnitDiscover)
          || (event.getType() == BWAPI::EventType::UnitMorph)
          || (event.getType() == BWAPI::EventType::UnitShow)
          || (event.getType() == BWAPI::EventType::UnitComplete))
        {
            BWAPI::Unit* unit = event.getUnit();
            BWAPI::UnitType unitType = unit->getType();
            engine->generateEvent(new CompleteUnitUpdateEvent(unit, unitType, getHealth(unit, unitType), BWAction::read(unit),
                                                              unit->getTilePosition(), unit->getPosition(), unit->getPlayer(), unit->getHatchery()));
        } else {
            engine->generateEvent(new BroodwarEvent(event));
            if (event.getType() == BWAPI::EventType::MatchEnd)
                return false;
        }
    }

    BWAPI::Player* neutral = BWAPI::Broodwar->neutral();
    for (auto unit : BWAPI::Broodwar->getAllUnits()) {
        BWAPI::UnitType unitType = unit->getType();
        if (unitType.isMineralField() || unitType.isRefinery() || (unitType == BWAPI::UnitTypes::Resource_Vespene_Geyser)) {
            engine->generateEvent(new ResourceUpdateEvent(unit, unit->getResources()));
        } else if (unit->getPlayer() != neutral) {
            engine->generateEvent(new SimpleUnitUpdateEvent(unit, unit->getPosition(), getHealth(unit, unitType), BWAction::read(unit)));
        }
    }

    return true;
}

void Blackboard::visitActionEvent(ActionEvent* event)
{
    auto it = actionMap.find(event->sender);
    if (it != actionMap.end()) {
        switch (event->type)
        {
            case ActionEvent::ActionTerminated:
            case ActionEvent::ActionFinished:
                it->second->setFinished();
                it->second->removeFinished(event->sender);
                removeItem(it->second);
                break;
            case ActionEvent::ActionFailed:
            case ActionEvent::ActionCleanedUp:
                it->second->setErrorState(event->sender);
                break;
        }
        actionMap.erase(it);
    }
    delete event->sender;
}

void Blackboard::visitAbstractActionEvent(AbstractActionEvent* event)
{
    auto it = actionMap.find(event->sender);
    if (it == actionMap.end())
        return;

    it->second->update(event);
}

void Blackboard::visitFrameEvent(FrameEvent* event)
{
    informations.lastUpdateTime     = event->currentTime;
    informations.currentMinerals    = event->currentMinerals;
    informations.currentGas         = event->currentGas;
    informations.collectedMinerals  = event->collectedMinerals;
    informations.collectedGas       = event->collectedGas;
}

void Blackboard::visitBroodwarEvent(BroodwarEvent* event)
{
    BWAPI::Event& e = event->event;
    if (e.getType() == BWAPI::EventType::UnitDestroy) {
        BWAPI::Unit* unit = e.getUnit();
        auto it = unitBoundaries.find(unit);
        if (it != unitBoundaries.end()) {
            delete it->second;
            unitBoundaries.erase(it);
        }
    }
    else if (e.getType() == BWAPI::EventType::MatchEnd) {
        getInformations()->isWinner=e.isWinner();
        LOG << "We "<<(e.isWinner()?"won.":"lost.");
    }
}

void Blackboard::visitUnitUpdateEvent(UnitUpdateEvent* event)
{
    auto it = unitBoundaries.find(event->unit);
    if (it == unitBoundaries.end())
        return;

    it->second->update(event);
}

bool connectToAssertionIsOn = true;

namespace
{
    struct DeativateConnectToAssertion
    {
        bool before;
        DeativateConnectToAssertion()
        {
            before = connectToAssertionIsOn;
            connectToAssertionIsOn = false;
        }
        ~DeativateConnectToAssertion()
        {
            connectToAssertionIsOn = before;
        }
    };

    template <class T>
    AbstractBoundaryItem* becomeBoundaryItem(AbstractBoundaryItem* oldItem, BWAPI::Unit* unit, BWAPI::UnitType unitType, BlackboardInformations* info)
    {
        if ((oldItem == NULL) || (typeid(*oldItem) != typeid(T))) {
            auto result = new T(unit, unitType, info);
            DeativateConnectToAssertion d;
            result->takeConnectionsFrom(oldItem);
            return result;
        } else {
            return oldItem;
        }
    }
}

void Blackboard::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event)
{
    AbstractBoundaryItem* oldItem = NULL;
    auto it = unitBoundaries.find(event->unit);
    if (it != unitBoundaries.end()) {
        oldItem = it->second;
    } else if ((event->unitType == BWAPI::UnitTypes::Zerg_Larva) && (event->hatchery != NULL)) {
        auto hatchery = dynamic_cast<OwnHatcheryBoundaryItem*>(lookupUnit(event->hatchery));
        if (hatchery != NULL) {
            oldItem = hatchery->removeFirstPlanedLarva();
            if (oldItem != NULL)
                oldItem->setUnit(event->unit);
        }
    }

    AbstractBoundaryItem* newItem = NULL;
    if (event->unitType.isResourceContainer()) {
        newItem = becomeBoundaryItem<ResourceBoundaryItem>(oldItem, event->unit, event->unitType, &informations);
    } else if (event->owner == self()) {
        if (   (event->unitType == BWAPI::UnitTypes::Zerg_Hatchery)
            || (event->unitType == BWAPI::UnitTypes::Zerg_Lair)
            || (event->unitType == BWAPI::UnitTypes::Zerg_Hive))
        {
            newItem = becomeBoundaryItem<OwnHatcheryBoundaryItem>(oldItem, event->unit, event->unitType, &informations);
        } else {
            newItem = becomeBoundaryItem<OwnUnitBoundaryItem>(oldItem, event->unit, event->unitType, &informations);
        }
    } else if (event->owner != neutral()) {
        newItem = becomeBoundaryItem<EnemyUnitBoundaryItem>(oldItem, event->unit, event->unitType, &informations);
    }

    if (newItem == oldItem) {
        oldItem = NULL;
    } else if (oldItem != NULL) {
        delete oldItem;
    }

    if (newItem != NULL) {
        unitBoundaries[event->unit] = newItem;
        newItem->update(event);
    } else {
        unitBoundaries.erase(event->unit);
    }
}

void Blackboard::visitFieldSeenEvent(FieldSeenEvent* event)
{
    informations.fieldSeen(event->tilePos, event->creep);
}
