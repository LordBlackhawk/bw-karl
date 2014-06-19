#include "plan-item.hpp"
#include "broodwar-plan-items.hpp"
#include "engine/abstract-action.hpp"
#include "engine/broodwar-events.hpp"
#include "utils/log.hpp"
#include <algorithm>

AbstractPort::AbstractPort()
    : estimatedTime(INFINITE_TIME)
{ }

AbstractPort::~AbstractPort()
{ }

AbstractPlanItem::AbstractPlanItem()
    : estimatedStartTime(INFINITE_TIME)
{ }

AbstractPlanItem::~AbstractPlanItem()
{ }

void AbstractPlanItem::updateEstimates()
{
    estimatedStartTime = START_TIME;
    for (auto it : ports)
        if (it->isRequirePort())
            estimatedStartTime = std::max(estimatedStartTime, it->estimatedTime);
}

Blackboard::Blackboard(AbstractExecutionEngine* e)
    : engine(e)
{ }

Blackboard::~Blackboard()
{ }

void Blackboard::addItem(AbstractPlanItem* item)
{
    items.push_back(item);
}

void Blackboard::removeItem(AbstractPlanItem* item)
{
    items.erase(std::remove(items.begin(), items.end(), item), items.end());
}

void Blackboard::addExpert(AbstractExpert* expert)
{
    experts.push_back(expert);
}

void Blackboard::removeExpert(AbstractExpert* expert)
{
    experts.erase(std::remove(experts.begin(), experts.end(), expert), experts.end());
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

void Blackboard::prepare()
{
    informations.self = BWAPI::Broodwar->self();

    for (auto it : experts)
        it->prepare();
}

void Blackboard::recalculateEstimatedTimes()
{
    for (auto it : items)
        if (!it->isActive())
            it->updateEstimates();
    std::sort(items.begin(), items.end(), PlanItemCompare());
}

void Blackboard::tick()
{
    // 1. Receive events
    AbstractEvent* event = engine->popEvent();
    while (event != NULL) {
        event->acceptVisitor(this);
        delete event;
        event = engine->popEvent();
    }

    // 2. Recalculate estimatedTimes
    recalculateEstimatedTimes();

    // 3. Experts may change the plan
    for (auto it : experts)
        it->tick(this);

    // 4. Execute actions which are planed soon
    const Time timeHorizont = getLastUpdateTime() + 10;
    for (auto it : items)
        if (!it->isActive() && (it->estimatedStartTime < timeHorizont))
            if (it->prepareForExecution(engine))
                it->estimatedStartTime = -1;
}

void Blackboard::visitActionEvent(ActionEvent* /*event*/)
{
    // ToDo
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
    if (e.getType() == BWAPI::EventType::UnitCreate) {
        BWAPI::Unit* unit = e.getUnit();
        if (unit->getPlayer() == self()) {
            LOG << "Own unit added: " << unit->getType().getName();
            auto item = new OwnUnitPlanItem(unit);
            addItem(item);
            unitBoundaries[unit] = item;
        }
    }
}

void Blackboard::visitUnitEvent(UnitEvent* event)
{
    auto it = unitBoundaries.find(event->unit);
    if (it == unitBoundaries.end())
        return;

    auto item = dynamic_cast<OwnUnitPlanItem*>(it->second);
    if (item == NULL)
        return;

    item->updateData(event->unitType, event->pos);
}
