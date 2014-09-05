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

AbstractBoundaryItem::AbstractBoundaryItem(BWAPI::Unit* u)
    : AbstractItem(u)
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

bool Blackboard::includeItem(AbstractPlanItem* item) const
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

AbstractBoundaryItem* Blackboard::lookupUnit(BWAPI::Unit* unit) const
{
    auto it = unitBoundaries.find(unit);
    return (it != unitBoundaries.end()) ? it->second : NULL;
}

AbstractPlanItem* Blackboard::create(BWAPI::UnitType ut)
{
    BWAPI::UnitType builderType = ut.whatBuilds().first;
    if (builderType.isWorker())
        return build(ut);
    if ((builderType == BWAPI::UnitTypes::Zerg_Larva) || (builderType.isBuilding() && (builderType.getRace() == BWAPI::Races::Zerg)))
        return morph(ut);
    assert(false && "Blackboard does not know how to create unit of this type.");
    return NULL;
}

BuildPlanItem* Blackboard::build(BWAPI::UnitType ut)
{
    auto result = new BuildPlanItem(&informations.fields, ut, BWAPI::TilePositions::Unknown);
    addItem(result);
    return result;
}

MorphUnitPlanItem* Blackboard::morph(BWAPI::UnitType ut)
{
    auto result = new MorphUnitPlanItem(ut);
    addItem(result);
    return result;
}

GatherMineralsPlanItem* Blackboard::gather(ProvideUnitPort* provider, ResourceBoundaryItem* m)
{
    auto result = new GatherMineralsPlanItem(m, provider);
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
    const Time timeHorizont = getLastUpdateTime() + 10;
    for (auto it : items)
        if (it->isPlanned() && (it->estimatedStartTime < timeHorizont))
    {
        AbstractAction* action = it->prepareForExecution(engine);
        if (action != NULL) {
            actionMap[action] = it;
            it->setActive();
        }
    }
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

void Blackboard::sendFrameEvent(AbstractExecutionEngine* engine)
{
    BWAPI::Player* self = BWAPI::Broodwar->self();
    engine->generateEvent(new FrameEvent(BWAPI::Broodwar->getFrameCount(), self->minerals(), self->gas()));

    for (auto event : BWAPI::Broodwar->getEvents()) {
        if ( (event.getType() == BWAPI::EventType::UnitCreate)
          || (event.getType() == BWAPI::EventType::UnitDiscover)
          || (event.getType() == BWAPI::EventType::UnitMorph)
          || (event.getType() == BWAPI::EventType::UnitShow)
          || (event.getType() == BWAPI::EventType::UnitComplete))
        {
            BWAPI::Unit* unit = event.getUnit();
            engine->generateEvent(new CompleteUnitUpdateEvent(unit, unit->getType(), unit->getTilePosition(), unit->getPosition(), unit->getPlayer()));
        } else {
            engine->generateEvent(new BroodwarEvent(event));
        }
    }

    BWAPI::Player* neutral = BWAPI::Broodwar->neutral();
    for (auto unit : BWAPI::Broodwar->getAllUnits()) {
        if (unit->getType().isMineralField()) {
            engine->generateEvent(new MineralUpdateEvent(unit, unit->getResources()));
        } else if (unit->getPlayer() != neutral) {
            engine->generateEvent(new SimpleUnitUpdateEvent(unit, unit->getPosition()));
        }
    }
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
    informations.lastUpdateTime  = event->currentTime;
    informations.currentMinerals = event->currentMinerals;
    informations.currentGas      = event->currentGas;
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
}

void Blackboard::visitUnitUpdateEvent(UnitUpdateEvent* event)
{
    auto it = unitBoundaries.find(event->unit);
    if (it == unitBoundaries.end())
        return;

    it->second->update(event);
}

void Blackboard::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event)
{
    auto it = unitBoundaries.find(event->unit);
    if (it != unitBoundaries.end()) {
        it->second->update(event);
        return;
    }

    AbstractBoundaryItem* item = NULL;
    if (event->unitType.isResourceContainer()) {
        item = new ResourceBoundaryItem(event->unit, event->unitType, &informations);
    } else if (event->owner == self()) {
        //LOG << "Own unit added: " << event->unitType.getName();
        item = new OwnUnitBoundaryItem(event->unit, event->unitType, &informations.fields);
    } else if (event->owner != neutral()) {
        //LOG << "Enemy unit added: " << event->unitType.getName();
        item = new EnemyUnitBoundaryItem(event->unit, event->unitType, &informations);
    }
    if (item != NULL) {
        unitBoundaries[event->unit] = item;
        item->update(event);
    }
}

void Blackboard::visitFieldSeenEvent(FieldSeenEvent* event)
{
    informations.fieldSeen(event->tilePos, event->creep);
}
