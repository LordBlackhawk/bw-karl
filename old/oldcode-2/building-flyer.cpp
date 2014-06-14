#include "building-flyer.hpp"
#include "container-helper.hpp"
#include "object-counter.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include <vector>

using namespace BWAPI;

#define THIS_DEBUG LOG

namespace
{
    struct BuildingFlyerPrecondition;
    std::vector<BuildingFlyerPrecondition*> list;

    struct BuildingFlyerPrecondition : public UnitPrecondition, public ObjectCounter<BuildingFlyerPrecondition>
    {
        enum StatusType { pending, commanded, waiting, finished };

        UnitPrecondition*               building;
        BuildingPositionPrecondition*   newpos;
        StatusType                      status;

        BuildingFlyerPrecondition(UnitPrecondition* b, BuildingPositionPrecondition* p)
            : UnitPrecondition(1, b->ut, Position(p->pos), UnitPrecondition::WithoutAddon), building(b), newpos(p), status(pending)
        {
            list.push_back(this);
        }

        ~BuildingFlyerPrecondition()
        {
            Containers::remove(list, this);

            release(building);
            release(newpos);
        }

        bool update()
        {
            switch (status)
            {
                case pending:
                {
                    double dis = building->pos.getDistance(Position(newpos->pos));
                    int timeNeeded = int(dis / ut.topSpeed());
                    if (updateTimePreconditions(this, timeNeeded, building, newpos)) {
                        lift();
                        time = Broodwar->getFrameCount() + timeNeeded;
                        THIS_DEBUG << "lifting building of type " << ut << ".";
                    }
                    break;
                }

                case commanded:
                    if (unit->isLifted()) {
                        land();
                        THIS_DEBUG << "landing building of type " << ut << ".";
                    }
                    break;

                case waiting:
                    if (!unit->isLifted()) {
                        status = finished;
                        time   = 0;
                        THIS_DEBUG << "building landed.";
                    }
                    break;

                case finished:
                    break;
            }
            return (status == finished);
        }

        void lift()
        {
            if (building != NULL) {
                unit = building->unit;
                release(building);
            }

            if (!unit->lift()) {
                auto err = Broodwar->getLastError();
                WARNING << "Building of type " << ut << " returns error if lifted: " << err.toString();
            }

            status = commanded;
        }

        void land()
        {
            if (!unit->land(newpos->pos)) {
                auto err = Broodwar->getLastError();
                WARNING << "Building of type " << ut << " returns error if landing: " << err.toString();
            }

            status = waiting;
        }

        const char* getStatusText() const
        {
            switch (status)
            {
                case pending:
                    return "pending";
                case commanded:
                    return "commanded";
                case waiting:
                    return "waiting";
                case finished:
                default:
                    return "finished";
            }
        }

        void onDrawPlan()
        {
            int x, y, width = 32*ut.tileWidth(), height = 32*ut.tileHeight();
            if (newpos != NULL) {
                Position p(newpos->pos);
                x = p.x();
                y = p.y();
            } else {
                Position p = unit->getPosition();
                x = p.x() - width/2;
                y = p.y() - height/2;
            }

            Broodwar->drawBoxMap(x, y, x + width, y + height, Colors::Green, false);
            Broodwar->drawTextMap(x+2, y+2,  "%s", ut.getName().c_str());
            Broodwar->drawTextMap(x+2, y+18, "%s", getStatusText());
        }
    };
}

UnitPrecondition* flyBuilding(UnitPrecondition* building, BuildingPositionPrecondition* pos)
{
    if ((building == NULL) || (pos == NULL)) {
        release(building);
        release(pos);
        return NULL;
    }
    if (!building->ut.isFlyingBuilding()) {
        release(building);
        release(pos);
        return NULL;
    }
    return new BuildingFlyerPrecondition(building, pos);
}

void BuildingFlyerCode::onMatchEnd()
{
    Containers::clear_and_delete(list);
}

void BuildingFlyerCode::onTick()
{
    Containers::remove_if(list, std::mem_fun(&BuildingFlyerPrecondition::update));
}

void BuildingFlyerCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    for (auto it : list)
        it->onDrawPlan();
}

void BuildingFlyerCode::onCheckMemoryLeaks()
{
    BuildingFlyerPrecondition::checkObjectsAlive();
}
