#pragma once

namespace
{
    template <class F>
    void callForEnergyTiles(const TilePosition& tp, F f)
    {
        //         76543210123456789
        //      -4 .................
        //      -3 ..xxxxxxxxxxxx...
        //      -2 .xxxxxxxxxxxxxx..
        //      -1 .xxxxxxxxxxxxxxx.
        //       0 .xxxxxxxxxxxxxxx.
        //       1 .xxxxxxxxxxxxxxx.
        //       2 .xxxxxxxxxxxxxxx.
        //       3 .xxxxxxxxxxxxxx..
        //       4 ..xxxxxxxxxxxx...
        //       5 .....xxxxxx......
        //       6 .................
        int xoff[] = { -5, -6, -6, -6, -6, -6, -6, -5, -2 };
        int xlen[] = { 12, 14, 15, 15, 15, 15, 14, 12,  6 };
        for (int dy=0; dy<9; ++dy) {
            int y = tp.y() - 3 + dy;
            if (y >= 0 && y < mapHeight)
                for (int dx=0; dx<xlen[dy]; ++dx)
            {
                int x = tp.x() + xoff[dy] + dx;
                if (x >= 0 && x < mapWidth) {
                    TilePosition p(x, y);
                    f(p);
                }
            }
        }
    }

    struct SetEnergyTime
    {
        void* obj;
        int newtime;
        SetEnergyTime(void* o, int nt)
            : obj(o), newtime(nt)
        { }
        void operator () (const TilePosition& pos) const
        {
            TileInformation& info = tileInformations[pos];
            if (info.energyobj == obj) {
                info.energytime = newtime;
            } else if (info.energytime > newtime) {
                info.energyobj  = obj;
                info.energytime = newtime;
            }
        }
    };

    void setEnergyTime(const TilePosition& pos, void* obj, int newtime)
    {
        callForEnergyTiles(pos, SetEnergyTime(obj, newtime));
    }

    /*
    int getPowerTime(const TilePosition& pos, const UnitType& type)
    {
        int x = 32 * pos.x() + 16 * type.tileWidth();
        int y = 32 * pos.y() + 16 * type.tileHeight();
        TilePosition tp(Position(x, y));
        TileInformation& info = tileInformations[tp];
        return info.energytime;
    }
    */

    struct RangeBuildingObserver;
    std::vector<RangeBuildingObserver*> rangebuildings;

    struct RangeBuildingObserver : public UnitObserver<RangeBuildingObserver>, public ObjectCounter<RangeBuildingObserver>
    {
        RangeBuildingObserver(UnitPrecondition* p)
            : UnitObserver<RangeBuildingObserver>(p)
        {
            rangebuildings.push_back(this);
        }

        void onRemoveFromList()
        {
            Containers::remove(rangebuildings, this);
        }

        void onUpdate()
        {
            setEnergyTime(TilePosition(pos), this, std::min(time, Precondition::Max-1));
        }

        void onFulfilled()
        {
            setEnergyTime(unit->getTilePosition(), unit, 0);
            /*
            TilePosition tp = unit->getTilePosition();
            int px = tp.x(), py = tp.y();
            std::stringstream stream;
            stream << "\t\t";
            for (int y=py-4; y<py+7; ++y) {
                for (int x=px-7; x<px+10; ++x)
                    stream << (Broodwar->hasPower(x, y) ? "x" : ".");
                stream << "\n\t\t";
            }
            LOG << "Energy at Frame " << Broodwar->getFrameCount() << "\n" << stream.str();
            */
        }
    };
}
