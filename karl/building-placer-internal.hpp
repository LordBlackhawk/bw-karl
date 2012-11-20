#pragma once

#include "arrangement.hpp"

namespace
{
    struct BuildingPositionInternal;
    std::vector<BuildingPositionInternal*> planedbuildings;

    struct BuildingPositionInternal : public BuildingPositionPrecondition, public ObjectCounter<BuildingPositionInternal>
    {
        Arrangement* arr;
        int          arr_id;

        BuildingPositionInternal(const BWAPI::UnitType& t, const BWAPI::TilePosition& p)
            : BuildingPositionPrecondition(0, t, p), arr(NULL), arr_id(0)
        {
            planedbuildings.push_back(this);
            reserveTiles(true);
            update();
        }

        BuildingPositionInternal(const BWAPI::UnitType& t, Arrangement* a, int id)
            : BuildingPositionPrecondition(1, t, BWAPI::TilePositions::Unknown), arr(a), arr_id(id)
        {
            planedbuildings.push_back(this);
            reserveTiles(true);
            update();
        }

        ~BuildingPositionInternal()
        {
            Containers::remove(planedbuildings, this);
            reserveTiles(false);
            if (arr != NULL) {
                arr->decRef();
                arr = NULL;
            }
        }

        virtual void renew()
        {
            if (arr != NULL) {
                LOG << "renew position called.";
                time = 1;
                arr->reset();
            }
        }

        void update()
        {
            if ((arr != NULL) && !(time == 0 && wishtime == 0)) {
                /*if (pos == BWAPI::Positions::Unknown)
                    LOG << "calling arr->onTick with unknown position.";*/
                BWAPI::TilePosition newpos = arr->onTick(arr_id);
                if (newpos != pos) {
                    reserveTiles(false);
                    pos = newpos;
                    reserveTiles(true);
                    time = (pos == BWAPI::Positions::Unknown) ? 1 : 0;
                }
            }
            if (pos != BWAPI::Positions::Unknown) {
                if (ut.requiresPsi())
                    time = getPowerTime(pos, ut);
            }
        }

        void reserveTiles(bool value)
        {
            if (pos == BWAPI::Positions::Unknown)
                return;

            int upperX = std::min(pos.x() + ut.tileWidth(), mapWidth);
            int upperY = std::min(pos.y() + ut.tileHeight(), mapHeight);
            for(int x=pos.x(); x<upperX; ++x)
                for(int y=pos.y(); y<upperY; ++y)
            {
                TileInformation& info = tileInformations[x][y];
                if (value) {
                    info.pre = this;
                } else if (info.pre == this) {
                    info.pre = NULL;
                }
            }
        }
    };
}
